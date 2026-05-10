/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <EndpointSecurityObserver.h>
#include <Log.h>

#include <chrono>

#if defined(__APPLE__)
#include <EndpointSecurity/EndpointSecurity.h>
#include <bsm/libbsm.h>
#endif


struct EndpointSecurityObserverState
{
#if defined(__APPLE__)
    es_client_t * client{nullptr};
#endif
};


EndpointSecurityObserver::EndpointSecurityObserver(const string & root)
    : root_(root),
      backendName_("endpointsec"s),
      state_(std::make_unique<EndpointSecurityObserverState>())
{}


EndpointSecurityObserver::~EndpointSecurityObserver()
{
    stop();
}


void
EndpointSecurityObserver::onMessage(const string & path, int op, int pid, int uid, uint64_t bytes)
{
    if (path.rfind(root_, 0) != 0)
        return;

    FsEvent ev;
    ev.fsPath = path;
    ev.pid = pid;
    ev.uid = uid;
    ev.bytes = bytes;
    ev.op = (op == 0) ? FsEvent::Op::Open : (op == 1 ? FsEvent::Op::Read : FsEvent::Op::Close);
    ev.ts = std::chrono::steady_clock::now();

    try {
        sink_(ev);
    } catch (...) {
        Log::Error("EndpointSecurityObserver: sink threw"s);
    }
}


#if defined(__APPLE__)


bool
EndpointSecurityObserver::start()
{
    if (running_.load())
        return true;
    if (!sink_) {
        Log::Error("EndpointSecurityObserver: sink not set"s);
        return false;
    }

    auto handler = ^(es_client_t * /*c*/, const es_message_t * msg) {
      if (msg->action_type != ES_ACTION_TYPE_NOTIFY)
          return;

      int op = -1;
      const es_file_t * file = nullptr;
      uint64_t bytes = 0;

      switch (msg->event_type) {
      case ES_EVENT_TYPE_NOTIFY_OPEN:
          op = 0;
          file = msg->event.open.file;
          break;
      case ES_EVENT_TYPE_NOTIFY_READ:
          op = 1;
          file = msg->event.read.source;
          break;
      case ES_EVENT_TYPE_NOTIFY_CLOSE:
          op = 2;
          file = msg->event.close.target;
          break;
      default:
          return;
      }

      if (!file)
          return;

      const string path(file->path.data, file->path.length);
      const auto pid = audit_token_to_pid(msg->process->audit_token);
      const auto uid = audit_token_to_euid(msg->process->audit_token);

      onMessage(path, op, static_cast<int>(pid), static_cast<int>(uid), bytes);
    };

    auto rc = es_new_client(&state_->client, handler);
    if (rc != ES_NEW_CLIENT_RESULT_SUCCESS) {
        switch (rc) {
        case ES_NEW_CLIENT_RESULT_ERR_NOT_ENTITLED:
            Log::Info("EndpointSecurityObserver: missing com.apple.developer.endpoint-security.client entitlement"s);
            break;
        case ES_NEW_CLIENT_RESULT_ERR_NOT_PRIVILEGED:
            Log::Info("EndpointSecurityObserver: must run as root"s);
            break;
        case ES_NEW_CLIENT_RESULT_ERR_NOT_PERMITTED:
            Log::Info("EndpointSecurityObserver: not permitted (TCC / Full Disk Access?)"s);
            break;
        default:
            Log::Error("EndpointSecurityObserver: es_new_client failed (rc="s + std::to_string(rc) + ")"s);
            break;
        }
        state_->client = nullptr;
        return false;
    }

    es_event_type_t events[] = {ES_EVENT_TYPE_NOTIFY_OPEN, ES_EVENT_TYPE_NOTIFY_CLOSE, ES_EVENT_TYPE_NOTIFY_READ};
    if (es_subscribe(state_->client, events, sizeof(events) / sizeof(events[0])) != ES_RETURN_SUCCESS) {
        Log::Error("EndpointSecurityObserver: es_subscribe failed"s);
        es_delete_client(state_->client);
        state_->client = nullptr;
        return false;
    }

    running_.store(true);
    Log::Info("EndpointSecurityObserver: started watching "s + root_);
    return true;
}


void
EndpointSecurityObserver::stop()
{
    if (!running_.load() && !state_->client)
        return;

    if (state_->client) {
        es_unsubscribe_all(state_->client);
        es_delete_client(state_->client);
        state_->client = nullptr;
    }
    running_.store(false);
}


#else  // non-Darwin


bool
EndpointSecurityObserver::start()
{
    Log::Error("EndpointSecurityObserver: not supported on this platform"s);
    return false;
}

void
EndpointSecurityObserver::stop()
{}


#endif
