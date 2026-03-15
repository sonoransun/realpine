/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <fstream>


class AuditLog
{
  public:

    using t_KvPair  = std::pair<std::string_view, std::string_view>;
    using t_KvPairs = std::initializer_list<t_KvPair>;


    static bool  initialize (const string & auditLogFileName);

    static void  shutdown ();

    // Record an audit event. Thread-safe, non-blocking (queues for async write).
    static void  record (std::string_view  eventType,
                         std::string_view  actor,
                         std::string_view  action,
                         t_KvPairs         details = {});


  private:

    struct t_AuditEntry {
        string  json;
    };

    static void  writerLoop ();
    static string  formatEntry (std::string_view  eventType,
                                std::string_view  actor,
                                std::string_view  action,
                                t_KvPairs         details);

    static std::queue<t_AuditEntry>   queue_s;
    static std::mutex                 queueMutex_s;
    static std::condition_variable    queueCv_s;
    static std::atomic<bool>          running_s;
    static std::thread                writerThread_s;
    static std::ofstream              file_s;

};
