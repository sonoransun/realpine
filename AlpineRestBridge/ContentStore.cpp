/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ContentStore.h>
#include <Log.h>
#include <ReadLock.h>
#include <WriteLock.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>

#include <dirent.h>
#include <sys/stat.h>

#if defined(ALPINE_PLATFORM_DARWIN)
#include <fcntl.h>
#include <sys/event.h>
#elif defined(ALPINE_PLATFORM_LINUX)
#include <poll.h>
#include <sys/inotify.h>
#endif


const std::unordered_map<string, string> ContentStore::mimeTypes_s = {{"mp4", "video/mp4"},
                                                                      {"m4v", "video/mp4"},
                                                                      {"mkv", "video/x-matroska"},
                                                                      {"avi", "video/x-msvideo"},
                                                                      {"mov", "video/quicktime"},
                                                                      {"wmv", "video/x-ms-wmv"},
                                                                      {"webm", "video/webm"},
                                                                      {"ts", "video/mp2t"},
                                                                      {"mpg", "video/mpeg"},
                                                                      {"mpeg", "video/mpeg"},
                                                                      {"flv", "video/x-flv"}};


bool
ContentStore::initialize(const string & mediaDirectory)
{
    mediaDirectory_ = mediaDirectory;

    struct stat dirStat;

    if (stat(mediaDirectory_.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
        Log::Error("ContentStore: Media directory does not exist: "s + mediaDirectory_);
        return false;
    }

    rescan();

    Log::Info("ContentStore: Scanned "s + std::to_string(items_.size()) + " video files from " + mediaDirectory_);

    startWatcher();

    return true;
}


void
ContentStore::shutdown()
{
    stopWatcher();
}


void
ContentStore::rescan()
{
    WriteLock guard(lock_);

    items_.clear();
    idToPath_.clear();
    nextId_ = 1;
    scanDirectory(mediaDirectory_);
    systemUpdateId_++;
}


bool
ContentStore::getItem(const string & id, MediaItem & item)
{
    ReadLock guard(lock_);

    auto pathIt = idToPath_.find(id);

    if (pathIt != idToPath_.end()) {
        auto itemIt = items_.find(pathIt->second);
        if (itemIt != items_.end()) {
            item = itemIt->second;
            return true;
        }
    }

    return false;
}


const ContentStore::ItemMap &
ContentStore::getAllItemsMap()
{
    return items_;
}


void
ContentStore::getAllItems(vector<MediaItem> & items)
{
    ReadLock guard(lock_);

    items.clear();
    items.reserve(items_.size());
    for (const auto & [path, item] : items_)
        items.push_back(item);
}


ulong
ContentStore::getItemCount()
{
    ReadLock guard(lock_);
    return items_.size();
}


ulong
ContentStore::getSystemUpdateId()
{
    ReadLock guard(lock_);
    return systemUpdateId_;
}


void
ContentStore::indexFile(const string & fullPath)
{
    struct stat fileStat;

    if (stat(fullPath.c_str(), &fileStat) != 0 || !S_ISREG(fileStat.st_mode))
        return;

    auto slashPos = fullPath.rfind('/');
    string name = (slashPos != string::npos) ? fullPath.substr(slashPos + 1) : fullPath;

    auto dotPos = name.rfind('.');

    if (dotPos == string::npos)
        return;

    string ext = name.substr(dotPos + 1);
    auto mimeIt = mimeTypes_s.find(ext);

    if (mimeIt == mimeTypes_s.end())
        return;

    WriteLock guard(lock_);

    auto existing = items_.find(fullPath);

    if (existing != items_.end()) {
        existing->second.fileSize = (ulong)fileStat.st_size;
        systemUpdateId_++;
        return;
    }

    MediaItem item;
    char idBuf[16];
    snprintf(idBuf, sizeof(idBuf), "media-%03lu", nextId_++);
    item.id = idBuf;
    item.path = fullPath;
    item.fileName = name;
    item.title = name.substr(0, dotPos);
    item.mimeType = mimeIt->second;
    item.fileSize = (ulong)fileStat.st_size;

    idToPath_[item.id] = fullPath;
    items_.emplace(fullPath, std::move(item));
    systemUpdateId_++;
}


void
ContentStore::removeFile(const string & fullPath)
{
    WriteLock guard(lock_);

    auto it = items_.find(fullPath);

    if (it != items_.end()) {
        idToPath_.erase(it->second.id);
        items_.erase(it);
        systemUpdateId_++;
    }
}


void
ContentStore::scanDirectory(const string & dir)
{
    struct DirCloser
    {
        void
        operator()(DIR * d) const
        {
            if (d)
                closedir(d);
        }
    };

    std::unique_ptr<DIR, DirCloser> dirp(opendir(dir.c_str()));

    if (!dirp) {
        Log::Error("ContentStore: Cannot open directory: "s + dir);
        return;
    }

    struct dirent * entry;

    while ((entry = readdir(dirp.get())) != nullptr) {
        string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        string fullPath = dir + "/" + name;
        struct stat fileStat;

        if (stat(fullPath.c_str(), &fileStat) != 0)
            continue;

        if (S_ISDIR(fileStat.st_mode)) {
            scanDirectory(fullPath);
            continue;
        }

        if (!S_ISREG(fileStat.st_mode))
            continue;

        auto dotPos = name.rfind('.');

        if (dotPos == string::npos)
            continue;

        string ext = name.substr(dotPos + 1);
        auto mimeIt = mimeTypes_s.find(ext);

        if (mimeIt == mimeTypes_s.end())
            continue;

        MediaItem item;
        char idBuf[16];
        snprintf(idBuf, sizeof(idBuf), "media-%03lu", nextId_++);
        item.id = idBuf;
        item.path = fullPath;
        item.fileName = name;
        item.title = name.substr(0, dotPos);
        item.mimeType = mimeIt->second;
        item.fileSize = (ulong)fileStat.st_size;

        idToPath_[item.id] = fullPath;
        items_.emplace(fullPath, std::move(item));
    }
}


// ============================================================================
//  Filesystem watcher
// ============================================================================


void
ContentStore::startWatcher()
{
    watcherStop_.store(false);
    watcher_ = std::make_unique<WatcherThread>(*this);
    watcher_->create();
    watcher_->resume();
}


void
ContentStore::stopWatcher()
{
    if (!watcher_) {
        return;
    }

    watcherStop_.store(true);

#if defined(ALPINE_PLATFORM_DARWIN)
    if (kqueueFd_ >= 0)
        close(kqueueFd_);
    kqueueFd_ = -1;
#elif defined(ALPINE_PLATFORM_LINUX)
    if (inotifyFd_ >= 0)
        close(inotifyFd_);
    inotifyFd_ = -1;
#endif

    watcher_->destroy();
    watcher_.reset();
}


void
ContentStore::WatcherThread::threadMain()
{
#if defined(ALPINE_PLATFORM_DARWIN)
    store_.watcherLoopKqueue();
#elif defined(ALPINE_PLATFORM_LINUX)
    store_.watcherLoopInotify();
#else
    store_.watcherLoopFallback();
#endif
}


#if defined(ALPINE_PLATFORM_DARWIN)

void
ContentStore::watcherLoopKqueue()
{
    kqueueFd_ = kqueue();

    if (kqueueFd_ < 0) {
        Log::Error("ContentStore: kqueue() failed, falling back to periodic scan"s);
        watcherLoopFallback();
        return;
    }

    int dirFd = open(mediaDirectory_.c_str(), O_RDONLY | O_EVTONLY | O_CLOEXEC);

    if (dirFd < 0) {
        Log::Error("ContentStore: Cannot open directory for kqueue watch: "s + mediaDirectory_);
        close(kqueueFd_);
        kqueueFd_ = -1;
        watcherLoopFallback();
        return;
    }

    struct kevent change;
    EV_SET(&change,
           dirFd,
           EVFILT_VNODE,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           NOTE_WRITE | NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE,
           0,
           nullptr);

    if (kevent(kqueueFd_, &change, 1, nullptr, 0, nullptr) < 0) {
        Log::Error("ContentStore: kevent registration failed"s);
        close(dirFd);
        close(kqueueFd_);
        kqueueFd_ = -1;
        watcherLoopFallback();
        return;
    }

    Log::Info("ContentStore: kqueue watcher started for "s + mediaDirectory_);

    auto lastFullScan = std::chrono::steady_clock::now();

    while (!watcherStop_.load()) {
        struct timespec timeout;
        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;

        struct kevent event;
        int nev = kevent(kqueueFd_, nullptr, 0, &event, 1, &timeout);

        if (watcherStop_.load())
            break;

        if (nev > 0) {
            rescan();
            Log::Debug("ContentStore: kqueue detected change, rescanned"s);
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastFullScan).count();

        if (elapsed >= fullScanIntervalSec_) {
            rescan();
            lastFullScan = now;
        }
    }

    close(dirFd);
}

#elif defined(ALPINE_PLATFORM_LINUX)

void
ContentStore::watcherLoopInotify()
{
    inotifyFd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);

    if (inotifyFd_ < 0) {
        Log::Error("ContentStore: inotify_init1() failed, falling back to periodic scan"s);
        watcherLoopFallback();
        return;
    }

    int wd = inotify_add_watch(
        inotifyFd_, mediaDirectory_.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);

    if (wd < 0) {
        Log::Error("ContentStore: inotify_add_watch failed for: "s + mediaDirectory_);
        close(inotifyFd_);
        inotifyFd_ = -1;
        watcherLoopFallback();
        return;
    }

    Log::Info("ContentStore: inotify watcher started for "s + mediaDirectory_);

    auto lastFullScan = std::chrono::steady_clock::now();
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

    while (!watcherStop_.load()) {
        struct pollfd pfd;
        pfd.fd = inotifyFd_;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 1000);

        if (watcherStop_.load())
            break;

        if (ret > 0 && (pfd.revents & POLLIN)) {
            ssize_t len = read(inotifyFd_, buf, sizeof(buf));

            if (len > 0) {
                rescan();
                Log::Debug("ContentStore: inotify detected change, rescanned"s);
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastFullScan).count();

        if (elapsed >= fullScanIntervalSec_) {
            rescan();
            lastFullScan = now;
        }
    }

    inotify_rm_watch(inotifyFd_, wd);
}

#endif


void
ContentStore::watcherLoopFallback()
{
    Log::Info("ContentStore: Using periodic scan fallback (every "s + std::to_string(fullScanIntervalSec_) + "s)");

    auto lastScan = std::chrono::steady_clock::now();

    while (!watcherStop_.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (watcherStop_.load())
            break;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastScan).count();

        if (elapsed >= fullScanIntervalSec_) {
            rescan();
            lastScan = now;
            Log::Debug("ContentStore: Periodic rescan completed"s);
        }
    }
}
