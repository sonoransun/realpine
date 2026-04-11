/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>


class WebhookDispatcher
{
  public:
    static void initialize();
    static void shutdown();

    // Queue a webhook delivery.  Thread-safe.
    static void dispatch(const string & callbackUrl, const string & jsonPayload, const string & eventType);

    // Configuration
    static void setSecret(const string & secret);
    static void setMaxRetries(int maxRetries);
    static void setTimeoutSeconds(int timeout);

    // Exposed for testing
    static string computeHmacSignature(const string & payload);

    struct t_UrlParts
    {
        string host;
        string port;
        string path;
        bool useTls{false};
    };

    static bool parseUrl(const string & url, t_UrlParts & parts);


  private:
    struct t_PendingWebhook
    {
        string callbackUrl;
        string jsonPayload;
        string eventType;
        int retryCount{0};
        std::chrono::steady_clock::time_point nextAttempt;
    };

    static bool deliverWebhook(const t_PendingWebhook & webhook);
    static void workerLoop();

    static std::queue<t_PendingWebhook> queue_s;
    static std::mutex queueMutex_s;
    static std::condition_variable queueCv_s;
    static string secret_s;
    static int maxRetries_s;
    static int timeoutSeconds_s;
    static std::atomic<bool> running_s;
    static std::thread workerThread_s;
};
