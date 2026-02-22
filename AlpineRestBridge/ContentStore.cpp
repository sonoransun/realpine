/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ContentStore.h>
#include <Log.h>

#include <cstdio>

#include <dirent.h>
#include <sys/stat.h>


const std::unordered_map<string, string>  ContentStore::mimeTypes_s = {
    {"mp4",  "video/mp4"},
    {"m4v",  "video/mp4"},
    {"mkv",  "video/x-matroska"},
    {"avi",  "video/x-msvideo"},
    {"mov",  "video/quicktime"},
    {"wmv",  "video/x-ms-wmv"},
    {"webm", "video/webm"},
    {"ts",   "video/mp2t"},
    {"mpg",  "video/mpeg"},
    {"mpeg", "video/mpeg"},
    {"flv",  "video/x-flv"}
};


bool
ContentStore::initialize (const string & mediaDirectory)
{
    mediaDirectory_ = mediaDirectory;

    struct stat dirStat;

    if (stat(mediaDirectory_.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
        Log::Error("ContentStore: Media directory does not exist: "s + mediaDirectory_);
        return false;
    }

    rescan();

    Log::Info("ContentStore: Scanned "s + std::to_string(items_.size()) +
              " video files from " + mediaDirectory_);

    return true;
}


void
ContentStore::rescan ()
{
    lock_.acquire();

    items_.clear();
    idIndex_.clear();
    nextId_ = 1;
    scanDirectory(mediaDirectory_);
    systemUpdateId_++;

    lock_.release();
}


bool
ContentStore::getItem (const string & id, MediaItem & item)
{
    lock_.acquire();

    auto it = idIndex_.find(id);

    if (it != idIndex_.end()) {
        item = items_[it->second];
        lock_.release();
        return true;
    }

    lock_.release();
    return false;
}


void
ContentStore::getAllItems (vector<MediaItem> & items)
{
    lock_.acquire();
    items = items_;
    lock_.release();
}


ulong
ContentStore::getItemCount ()
{
    lock_.acquire();
    ulong count = items_.size();
    lock_.release();
    return count;
}


ulong
ContentStore::getSystemUpdateId ()
{
    lock_.acquire();
    ulong id = systemUpdateId_;
    lock_.release();
    return id;
}


void
ContentStore::scanDirectory (const string & dir)
{
    DIR * dirp = opendir(dir.c_str());

    if (!dirp) {
        Log::Error("ContentStore: Cannot open directory: "s + dir);
        return;
    }

    struct dirent * entry;

    while ((entry = readdir(dirp)) != nullptr)
    {
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
        item.id       = idBuf;
        item.path     = fullPath;
        item.fileName = name;
        item.title    = name.substr(0, dotPos);
        item.mimeType = mimeIt->second;
        item.fileSize = (ulong)fileStat.st_size;

        idIndex_[item.id] = items_.size();
        items_.push_back(std::move(item));
    }

    closedir(dirp);
}
