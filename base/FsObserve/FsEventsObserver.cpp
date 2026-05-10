/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <FsEventsObserver.h>
#include <Log.h>

#include <chrono>

#if defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#endif


// Internal state held out-of-line so the header stays platform-agnostic.
struct FsEventsObserverState
{
#if defined(__APPLE__)
    FSEventStreamRef stream{nullptr};
    dispatch_queue_t queue{nullptr};
#endif
};


FsEventsObserver::FsEventsObserver(const string & root)
    : root_(root),
      backendName_("fsevents"s),
      state_(std::make_unique<FsEventsObserverState>())
{}


FsEventsObserver::~FsEventsObserver()
{
    stop();
}


void
FsEventsObserver::onPath(const string & path, uint32_t flags)
{
#if defined(__APPLE__)
    if (path.rfind(root_, 0) != 0)
        return;

    constexpr uint32_t kModifiedMask = kFSEventStreamEventFlagItemModified;
    if (!(flags & kModifiedMask))
        return;

    FsEvent ev;
    ev.fsPath = path;
    ev.ts = std::chrono::steady_clock::now();

    ev.op = FsEvent::Op::Open;
    try {
        sink_(ev);
    } catch (...) {
        Log::Error("FsEventsObserver: sink threw on Open"s);
    }

    ev.op = FsEvent::Op::Close;
    try {
        sink_(ev);
    } catch (...) {
        Log::Error("FsEventsObserver: sink threw on Close"s);
    }
#else
    (void)path;
    (void)flags;
#endif
}


#if defined(__APPLE__)


namespace {

void
fsEventsCallback(ConstFSEventStreamRef /*stream*/,
                 void * clientInfo,
                 size_t numEvents,
                 void * eventPaths,
                 const FSEventStreamEventFlags eventFlags[],
                 const FSEventStreamEventId /*eventIds*/[])
{
    auto * self = static_cast<FsEventsObserver *>(clientInfo);
    auto * paths = static_cast<char **>(eventPaths);

    for (size_t i = 0; i < numEvents; ++i)
        self->onPath(string(paths[i]), static_cast<uint32_t>(eventFlags[i]));
}

}  // namespace


bool
FsEventsObserver::start()
{
    if (running_.load())
        return true;
    if (!sink_) {
        Log::Error("FsEventsObserver: sink not set"s);
        return false;
    }

    CFStringRef rootCf = CFStringCreateWithCString(kCFAllocatorDefault, root_.c_str(), kCFStringEncodingUTF8);
    CFArrayRef pathsToWatch = CFArrayCreate(kCFAllocatorDefault, (const void **)&rootCf, 1, &kCFTypeArrayCallBacks);

    FSEventStreamContext ctx{};
    ctx.info = this;

    state_->stream = FSEventStreamCreate(kCFAllocatorDefault,
                                         &fsEventsCallback,
                                         &ctx,
                                         pathsToWatch,
                                         kFSEventStreamEventIdSinceNow,
                                         /* latency */ 0.5,
                                         kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer |
                                             kFSEventStreamCreateFlagWatchRoot);

    CFRelease(pathsToWatch);
    CFRelease(rootCf);

    if (!state_->stream) {
        Log::Error("FsEventsObserver: FSEventStreamCreate failed"s);
        return false;
    }

    state_->queue = dispatch_queue_create("com.alpine.fsevents", DISPATCH_QUEUE_SERIAL);
    FSEventStreamSetDispatchQueue(state_->stream, state_->queue);

    if (!FSEventStreamStart(state_->stream)) {
        Log::Error("FsEventsObserver: FSEventStreamStart failed"s);
        FSEventStreamInvalidate(state_->stream);
        FSEventStreamRelease(state_->stream);
        state_->stream = nullptr;
        dispatch_release(state_->queue);
        state_->queue = nullptr;
        return false;
    }

    running_.store(true);
    Log::Info("FsEventsObserver: started watching "s + root_);
    return true;
}


void
FsEventsObserver::stop()
{
    if (!running_.load() && !state_->stream)
        return;

    if (state_->stream) {
        FSEventStreamStop(state_->stream);
        FSEventStreamInvalidate(state_->stream);
        FSEventStreamRelease(state_->stream);
        state_->stream = nullptr;
    }
    if (state_->queue) {
        dispatch_release(state_->queue);
        state_->queue = nullptr;
    }
    running_.store(false);
}


#else  // non-Darwin: stubbed start/stop (the stub source supplants this when the backend is disabled at build time)


bool
FsEventsObserver::start()
{
    Log::Error("FsEventsObserver: not supported on this platform"s);
    return false;
}

void
FsEventsObserver::stop()
{}


#endif
