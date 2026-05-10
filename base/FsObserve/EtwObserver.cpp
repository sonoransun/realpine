/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <EtwObserver.h>
#include <Log.h>

#include <chrono>
#include <cstring>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <evntcons.h>
#include <evntrace.h>
#include <tdh.h>
#endif


namespace {

constexpr wchar_t kSessionName[] = L"AlpineFsObserve";

// Microsoft-Windows-Kernel-File provider GUID
// {EDD08927-9CC4-4E65-B970-C2560FB5C289}
#if defined(_WIN32)
constexpr GUID kKernelFileGuid = {0xEDD08927, 0x9CC4, 0x4E65, {0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89}};
#endif

// Microsoft-Windows-Kernel-File event IDs of interest:
//   12 = Create
//   14 = Close
//   15 = Read
constexpr uint16_t kEventCreate = 12;
constexpr uint16_t kEventClose = 14;
constexpr uint16_t kEventRead = 15;

}  // namespace


struct EtwObserverState
{
#if defined(_WIN32)
    EVENT_TRACE_PROPERTIES * sessionProps{nullptr};
    TRACEHANDLE sessionHandle{0};
    TRACEHANDLE traceHandle{INVALID_PROCESSTRACE_HANDLE};
#endif
};


EtwObserver::EtwObserver(const string & root)
    : root_(root),
      backendName_("etw"s),
      state_(std::make_unique<EtwObserverState>())
{}


EtwObserver::~EtwObserver()
{
    stop();
}


void
EtwObserver::rememberFile(uint64_t fileObject, const string & path)
{
    std::lock_guard<std::mutex> g(pathMutex_);
    filePaths_[fileObject] = path;
}


bool
EtwObserver::lookupFile(uint64_t fileObject, string & path)
{
    std::lock_guard<std::mutex> g(pathMutex_);
    auto it = filePaths_.find(fileObject);
    if (it == filePaths_.end())
        return false;
    path = it->second;
    return true;
}


void
EtwObserver::forgetFile(uint64_t fileObject)
{
    std::lock_guard<std::mutex> g(pathMutex_);
    filePaths_.erase(fileObject);
}


#if defined(_WIN32)


namespace {

string
wideToUtf8(const wchar_t * data, size_t lenChars)
{
    if (!data || lenChars == 0)
        return string();
    int needed = ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lenChars), nullptr, 0, nullptr, nullptr);
    string out(static_cast<size_t>(needed), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lenChars), out.data(), needed, nullptr, nullptr);
    return out;
}


// Pull a single property by name from an event record using TDH.  Returns
// the raw bytes; caller decides how to interpret.  Best-effort: returns
// false when the property is missing or TDH fails.
bool
getProperty(PEVENT_RECORD rec, PCWSTR propertyName, std::vector<BYTE> & out)
{
    PROPERTY_DATA_DESCRIPTOR pdd{};
    pdd.PropertyName = reinterpret_cast<ULONGLONG>(propertyName);
    pdd.ArrayIndex = ULONG_MAX;

    ULONG sz = 0;
    if (TdhGetPropertySize(rec, 0, nullptr, 1, &pdd, &sz) != ERROR_SUCCESS || sz == 0)
        return false;

    out.assign(sz, 0);
    if (TdhGetProperty(rec, 0, nullptr, 1, &pdd, sz, out.data()) != ERROR_SUCCESS)
        return false;
    return true;
}


// Common per-event handler stash; the WINAPI callback only has a `void *`
// UserContext from EVENT_TRACE_LOGFILE::Context, which we point at the
// EtwObserver instance.
void WINAPI
eventRecordCallback(PEVENT_RECORD rec)
{
    if (!rec || !rec->UserContext)
        return;
    auto * self = static_cast<EtwObserver *>(rec->UserContext);

    if (!IsEqualGUID(rec->EventHeader.ProviderId, kKernelFileGuid))
        return;

    const uint16_t id = rec->EventHeader.EventDescriptor.Id;
    if (id != kEventCreate && id != kEventClose && id != kEventRead)
        return;

    // Extract FileObject (uint64) — present on Create/Close/Read.
    uint64_t fileObject = 0;
    {
        std::vector<BYTE> bytes;
        if (getProperty(rec, L"FileObject", bytes) && bytes.size() >= sizeof(uint64_t))
            std::memcpy(&fileObject, bytes.data(), sizeof(uint64_t));
    }

    // Pid: prefer the EventHeader.ProcessId (always present).
    int pid = static_cast<int>(rec->EventHeader.ProcessId);

    // IoSize is only present on Read.
    uint64_t bytes = 0;
    if (id == kEventRead) {
        std::vector<BYTE> raw;
        if (getProperty(rec, L"IOSize", raw) && raw.size() >= sizeof(uint32_t)) {
            uint32_t v32;
            std::memcpy(&v32, raw.data(), sizeof(uint32_t));
            bytes = v32;
        }
    }

    // FileName is present on Create; cache for later Close/Read lookups.
    string path;
    if (id == kEventCreate) {
        std::vector<BYTE> nameBytes;
        if (getProperty(rec, L"FileName", nameBytes) && nameBytes.size() >= sizeof(wchar_t)) {
            const auto * w = reinterpret_cast<const wchar_t *>(nameBytes.data());
            const size_t maxChars = nameBytes.size() / sizeof(wchar_t);
            size_t lenChars = 0;
            while (lenChars < maxChars && w[lenChars] != L'\0')
                ++lenChars;
            path = wideToUtf8(w, lenChars);
            if (!path.empty() && fileObject != 0)
                self->rememberFile(fileObject, path);
        }
    } else {
        if (!self->lookupFile(fileObject, path))
            return;  // unknown file object; can't attribute
    }

    if (path.empty() || path.rfind(self->root_, 0) != 0)
        return;

    FsEvent ev;
    ev.fsPath = path;
    ev.pid = pid;
    ev.bytes = bytes;
    ev.ts = std::chrono::steady_clock::now();

    if (id == kEventCreate)
        ev.op = FsEvent::Op::Open;
    else if (id == kEventRead)
        ev.op = FsEvent::Op::Read;
    else
        ev.op = FsEvent::Op::Close;

    try {
        self->sink_(ev);
    } catch (...) {
        Log::Error("EtwObserver: sink threw"s);
    }

    if (id == kEventClose)
        self->forgetFile(fileObject);
}

}  // namespace


bool
EtwObserver::start()
{
    if (running_.load())
        return true;
    if (!sink_) {
        Log::Error("EtwObserver: sink not set"s);
        return false;
    }

    constexpr ULONG kBufSize = static_cast<ULONG>(sizeof(EVENT_TRACE_PROPERTIES) + sizeof(kSessionName));
    state_->sessionProps =
        static_cast<EVENT_TRACE_PROPERTIES *>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, kBufSize));
    if (!state_->sessionProps) {
        Log::Error("EtwObserver: HeapAlloc failed"s);
        return false;
    }

    state_->sessionProps->Wnode.BufferSize = kBufSize;
    state_->sessionProps->Wnode.ClientContext = 1;  // QPC
    state_->sessionProps->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    state_->sessionProps->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    state_->sessionProps->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    ULONG rc = ::StartTraceW(&state_->sessionHandle, kSessionName, state_->sessionProps);
    if (rc == ERROR_ALREADY_EXISTS) {
        // Stop a stale session left from a prior crashed instance and retry.
        ::ControlTraceW(0, kSessionName, state_->sessionProps, EVENT_TRACE_CONTROL_STOP);
        rc = ::StartTraceW(&state_->sessionHandle, kSessionName, state_->sessionProps);
    }
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_ACCESS_DENIED) {
            Log::Info("EtwObserver: ERROR_ACCESS_DENIED — process needs admin or membership in "
                      "'Performance Log Users'"s);
        } else {
            Log::Error("EtwObserver: StartTraceW failed (rc="s + std::to_string(rc) + ")"s);
        }
        ::HeapFree(::GetProcessHeap(), 0, state_->sessionProps);
        state_->sessionProps = nullptr;
        return false;
    }

    rc = ::EnableTraceEx2(state_->sessionHandle,
                          &kKernelFileGuid,
                          EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                          TRACE_LEVEL_INFORMATION,
                          /*MatchAny*/ 0x80,
                          /*MatchAll*/ 0,
                          /*Timeout*/ 0,
                          nullptr);
    if (rc != ERROR_SUCCESS) {
        Log::Error("EtwObserver: EnableTraceEx2 failed (rc="s + std::to_string(rc) + ")"s);
        ::ControlTraceW(state_->sessionHandle, kSessionName, state_->sessionProps, EVENT_TRACE_CONTROL_STOP);
        ::HeapFree(::GetProcessHeap(), 0, state_->sessionProps);
        state_->sessionProps = nullptr;
        return false;
    }

    worker_ = std::make_unique<WorkerThread>(*this);
    if (!worker_->create()) {
        Log::Error("EtwObserver: failed to start worker thread"s);
        ::ControlTraceW(state_->sessionHandle, kSessionName, state_->sessionProps, EVENT_TRACE_CONTROL_STOP);
        ::HeapFree(::GetProcessHeap(), 0, state_->sessionProps);
        state_->sessionProps = nullptr;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("EtwObserver: started watching "s + root_);
    return true;
}


void
EtwObserver::stop()
{
    if (!running_.load() && !worker_)
        return;

    if (state_->sessionHandle) {
        // Stopping the session causes ProcessTrace to return.
        ::ControlTraceW(state_->sessionHandle, kSessionName, state_->sessionProps, EVENT_TRACE_CONTROL_STOP);
        state_->sessionHandle = 0;
    }
    if (state_->traceHandle != INVALID_PROCESSTRACE_HANDLE) {
        ::CloseTrace(state_->traceHandle);
        state_->traceHandle = INVALID_PROCESSTRACE_HANDLE;
    }

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (state_->sessionProps) {
        ::HeapFree(::GetProcessHeap(), 0, state_->sessionProps);
        state_->sessionProps = nullptr;
    }

    {
        std::lock_guard<std::mutex> g(pathMutex_);
        filePaths_.clear();
    }

    running_.store(false);
}


void
EtwObserver::WorkerThread::threadMain()
{
    owner_.processLoop();
}


void
EtwObserver::processLoop()
{
    EVENT_TRACE_LOGFILEW log{};
    log.LoggerName = const_cast<LPWSTR>(kSessionName);
    log.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    log.EventRecordCallback = &eventRecordCallback;
    log.Context = this;

    state_->traceHandle = ::OpenTraceW(&log);
    if (state_->traceHandle == INVALID_PROCESSTRACE_HANDLE) {
        Log::Error("EtwObserver: OpenTraceW failed (gle="s + std::to_string(::GetLastError()) + ")"s);
        return;
    }

    // ProcessTrace blocks until the session is stopped (by stop()).
    ULONG rc = ::ProcessTrace(&state_->traceHandle, 1, nullptr, nullptr);
    if (rc != ERROR_SUCCESS && rc != ERROR_CANCELLED) {
        Log::Error("EtwObserver: ProcessTrace returned "s + std::to_string(rc));
    }
}


#else  // non-Windows: stubbed (real implementation only on Windows)


bool
EtwObserver::start()
{
    Log::Error("EtwObserver: not supported on this platform"s);
    return false;
}

void
EtwObserver::stop()
{}

void
EtwObserver::WorkerThread::threadMain()
{}

void
EtwObserver::processLoop()
{}


#endif
