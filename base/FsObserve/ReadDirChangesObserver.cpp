/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <ReadDirChangesObserver.h>

#include <chrono>
#include <cstring>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif


struct ReadDirChangesObserverState
{
#if defined(_WIN32)
    HANDLE dir{INVALID_HANDLE_VALUE};
    HANDLE evt{nullptr};
    OVERLAPPED ov{};
    static constexpr DWORD kBufBytes = 32 * 1024;
    alignas(DWORD) BYTE buf[kBufBytes]{};
#endif
};


ReadDirChangesObserver::ReadDirChangesObserver(const string & root)
    : root_(root),
      backendName_("readdirchanges"s),
      state_(std::make_unique<ReadDirChangesObserverState>())
{}


ReadDirChangesObserver::~ReadDirChangesObserver()
{
    stop();
}


#if defined(_WIN32)


namespace {

// Convert a UTF-8 std::string to a Windows UTF-16 std::wstring.
std::wstring
utf8ToWide(const string & s)
{
    if (s.empty())
        return std::wstring();
    int needed = ::MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(needed), L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), needed);
    return out;
}


// Convert a UTF-16 substring (length in WCHARs) to UTF-8.
string
wideToUtf8(const wchar_t * data, size_t lenChars)
{
    if (lenChars == 0)
        return string();
    int needed = ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lenChars), nullptr, 0, nullptr, nullptr);
    string out(static_cast<size_t>(needed), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, data, static_cast<int>(lenChars), out.data(), needed, nullptr, nullptr);
    return out;
}

}  // namespace


bool
ReadDirChangesObserver::start()
{
    if (running_.load())
        return true;
    if (!sink_) {
        Log::Error("ReadDirChangesObserver: sink not set"s);
        return false;
    }

    auto wroot = utf8ToWide(root_);
    state_->dir = ::CreateFileW(wroot.c_str(),
                                FILE_LIST_DIRECTORY,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                nullptr);
    if (state_->dir == INVALID_HANDLE_VALUE) {
        Log::Error("ReadDirChangesObserver: CreateFileW failed for "s + root_ + " (gle="s +
                   std::to_string(::GetLastError()) + ")"s);
        return false;
    }

    state_->evt = ::CreateEventW(nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, nullptr);
    if (!state_->evt) {
        Log::Error("ReadDirChangesObserver: CreateEventW failed"s);
        ::CloseHandle(state_->dir);
        state_->dir = INVALID_HANDLE_VALUE;
        return false;
    }
    std::memset(&state_->ov, 0, sizeof(state_->ov));
    state_->ov.hEvent = state_->evt;

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("ReadDirChangesObserver: failed to start worker thread"s);
        ::CloseHandle(state_->evt);
        state_->evt = nullptr;
        ::CloseHandle(state_->dir);
        state_->dir = INVALID_HANDLE_VALUE;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("ReadDirChangesObserver: started watching "s + root_);
    return true;
}


void
ReadDirChangesObserver::stop()
{
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (state_->dir != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(state_->dir, &state_->ov);
        ::SetEvent(state_->evt);  // unblock the wait in the worker
    }

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (state_->evt) {
        ::CloseHandle(state_->evt);
        state_->evt = nullptr;
    }
    if (state_->dir != INVALID_HANDLE_VALUE) {
        ::CloseHandle(state_->dir);
        state_->dir = INVALID_HANDLE_VALUE;
    }

    running_.store(false);
}


void
ReadDirChangesObserver::WorkerThread::threadMain()
{
    owner_.watchLoop();
}


void
ReadDirChangesObserver::watchLoop()
{
    constexpr DWORD kFilter = FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE |
                              FILE_NOTIFY_CHANGE_FILE_NAME;

    while (!stopRequested_.load()) {
        DWORD bytesReturned = 0;
        BOOL ok = ::ReadDirectoryChangesW(state_->dir,
                                          state_->buf,
                                          ReadDirChangesObserverState::kBufBytes,
                                          /*bWatchSubtree*/ TRUE,
                                          kFilter,
                                          &bytesReturned,
                                          &state_->ov,
                                          nullptr);
        if (!ok) {
            Log::Error("ReadDirChangesObserver: ReadDirectoryChangesW failed (gle="s +
                       std::to_string(::GetLastError()) + ")"s);
            break;
        }

        // Wait with a timeout so we can poll stopRequested_.
        DWORD waitRc = ::WaitForSingleObject(state_->evt, 250);
        if (stopRequested_.load())
            break;
        if (waitRc == WAIT_TIMEOUT)
            continue;
        if (waitRc != WAIT_OBJECT_0)
            break;

        DWORD got = 0;
        if (!::GetOverlappedResult(state_->dir, &state_->ov, &got, FALSE))
            continue;
        ::ResetEvent(state_->evt);
        if (got == 0)
            continue;

        // Walk the linked list of FILE_NOTIFY_INFORMATION records.
        BYTE * cursor = state_->buf;
        for (;;) {
            auto * info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(cursor);

            if (info->Action == FILE_ACTION_MODIFIED) {
                const size_t lenChars = info->FileNameLength / sizeof(WCHAR);
                string name = wideToUtf8(info->FileName, lenChars);
                string fullPath = root_ + "\\"s + name;

                FsEvent ev;
                ev.fsPath = fullPath;
                ev.ts = std::chrono::steady_clock::now();

                ev.op = FsEvent::Op::Open;
                try {
                    sink_(ev);
                } catch (...) {
                    Log::Error("ReadDirChangesObserver: sink threw on Open"s);
                }

                ev.op = FsEvent::Op::Close;
                try {
                    sink_(ev);
                } catch (...) {
                    Log::Error("ReadDirChangesObserver: sink threw on Close"s);
                }
            }

            if (info->NextEntryOffset == 0)
                break;
            cursor += info->NextEntryOffset;
        }
    }
}


#else  // non-Windows: header members exist but the implementation is stubbed


bool
ReadDirChangesObserver::start()
{
    Log::Error("ReadDirChangesObserver: not supported on this platform"s);
    return false;
}

void
ReadDirChangesObserver::stop()
{}

void
ReadDirChangesObserver::WorkerThread::threadMain()
{}

void
ReadDirChangesObserver::watchLoop()
{}


#endif
