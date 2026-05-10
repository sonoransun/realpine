/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// eBPF/fsprobe-style observation.  This class is the *parent* half of the
/// design: the actual kprobe attachment runs in the privileged
/// `alpine-fsprobe` sidecar binary.  The parent spawns the sidecar with a
/// shared UDS, reads length-prefixed FsEvent frames, and forwards them to
/// its sink.
///
/// Wire format (little-endian, native ints):
///   u32 frameLen
///   u8  op          (0=Open, 1=Read, 2=Close)
///   u32 pid
///   i32 uid
///   u64 bytes
///   u64 durationNs
///   u32 pathLen
///   char path[pathLen]


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>
#include <sys/types.h>


class EbpfObserver : public FsObserver
{
  public:
    explicit EbpfObserver(const string & root);
    ~EbpfObserver() override;


    const string &
    root() const override
    {
        return root_;
    }
    const string &
    backendName() const override
    {
        return backendName_;
    }
    void
    setSink(t_Sink sink) override
    {
        sink_ = std::move(sink);
    }
    bool start() override;
    void stop() override;
    bool
    isRunning() const override
    {
        return running_.load();
    }


    // Override the path the parent uses to launch the sidecar; default is
    // `alpine-fsprobe` resolved via $PATH or alongside the running binary.
    //
    void
    setSidecarPath(const string & path)
    {
        sidecarPath_ = path;
    }


  private:
    class WorkerThread : public AutoThread
    {
      public:
        explicit WorkerThread(EbpfObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        EbpfObserver & owner_;
    };


    bool spawnSidecar();
    void readLoop();


    string root_;
    string backendName_;
    string sidecarPath_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    int parentFd_{-1};
    pid_t childPid_{-1};
};
