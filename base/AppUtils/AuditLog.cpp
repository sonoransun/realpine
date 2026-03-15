/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuditLog.h>
#include <Log.h>
#include <chrono>
#include <format>


std::queue<AuditLog::t_AuditEntry>   AuditLog::queue_s;
std::mutex                           AuditLog::queueMutex_s;
std::condition_variable              AuditLog::queueCv_s;
std::atomic<bool>                    AuditLog::running_s{false};
std::thread                          AuditLog::writerThread_s;
std::ofstream                        AuditLog::file_s;


bool
AuditLog::initialize (const string & auditLogFileName)
{
    file_s.open(auditLogFileName, std::ios::app);
    if (!file_s.is_open()) {
        Log::Error("AuditLog: failed to open audit log file: {}", auditLogFileName);
        return false;
    }

    running_s = true;
    writerThread_s = std::thread(writerLoop);

    Log::Info("AuditLog initialized: {}", auditLogFileName);
    return true;
}


void
AuditLog::shutdown ()
{
    if (!running_s) {
        return;
    }

    running_s = false;
    queueCv_s.notify_all();

    if (writerThread_s.joinable()) {
        writerThread_s.join();
    }

    // Flush remaining entries
    std::lock_guard lock(queueMutex_s);
    while (!queue_s.empty()) {
        file_s << queue_s.front().json << "\n";
        queue_s.pop();
    }
    file_s.flush();
    file_s.close();
}


void
AuditLog::record (std::string_view  eventType,
                  std::string_view  actor,
                  std::string_view  action,
                  t_KvPairs         details)
{
    if (!running_s) {
        return;
    }

    auto entry = t_AuditEntry{formatEntry(eventType, actor, action, details)};

    {
        std::lock_guard lock(queueMutex_s);
        queue_s.push(std::move(entry));
    }
    queueCv_s.notify_one();
}


void
AuditLog::writerLoop ()
{
    while (running_s) {
        std::unique_lock lock(queueMutex_s);
        queueCv_s.wait(lock, [] {
            return !queue_s.empty() || !running_s;
        });

        // Drain all pending entries under one lock acquisition
        while (!queue_s.empty()) {
            auto entry = std::move(queue_s.front());
            queue_s.pop();
            lock.unlock();

            file_s << entry.json << "\n";

            lock.lock();
        }

        file_s.flush();
    }
}


string
AuditLog::formatEntry (std::string_view  eventType,
                       std::string_view  actor,
                       std::string_view  action,
                       t_KvPairs         details)
{
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    // Build JSON-lines format entry
    string json = std::format(
        R"({{"timestamp":{},"event":"{}","actor":"{}","action":"{}")",
        epoch, eventType, actor, action);

    for (const auto & [k, v] : details) {
        json += std::format(",\"{}\":\"{}\"", k, v);
    }

    json += "}";
    return json;
}
