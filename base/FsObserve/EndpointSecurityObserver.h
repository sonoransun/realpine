/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// macOS EndpointSecurity-based filesystem observation.  Provides real
/// per-process pid/uid attribution for ES_EVENT_TYPE_NOTIFY_OPEN /
/// ES_EVENT_TYPE_NOTIFY_CLOSE / ES_EVENT_TYPE_NOTIFY_READ.
///
/// Runtime requirements:
///   - Binary must be code-signed with the
///     com.apple.developer.endpoint-security.client entitlement
///     (Apple Developer Program approval required).
///   - Must run as root.
///
/// When `es_new_client` returns ES_NEW_CLIENT_RESULT_ERR_NOT_ENTITLED
/// (the typical failure mode for unsigned local builds), `start()`
/// returns false and FsObserverFactory falls back to the next backend.


#pragma once
#include <FsObserver.h>
#include <atomic>
#include <memory>


// Opaque to keep the EndpointSecurity headers out of the public header.
struct EndpointSecurityObserverState;


class EndpointSecurityObserver : public FsObserver
{
  public:
    explicit EndpointSecurityObserver(const string & root);
    ~EndpointSecurityObserver() override;


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


  private:
    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<EndpointSecurityObserverState> state_;
    std::atomic<bool> running_{false};


    // Trampoline used by the ES message handler (defined in .cpp on macOS).
    void onMessage(const string & path, int op, int pid, int uid, uint64_t bytes);
};
