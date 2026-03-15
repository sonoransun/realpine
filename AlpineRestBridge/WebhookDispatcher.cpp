/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <WebhookDispatcher.h>
#include <Configuration.h>
#include <SafeParse.h>
#include <Log.h>

#include <asio.hpp>

#ifdef ALPINE_TLS_ENABLED
#include <openssl/hmac.h>
#include <openssl/evp.h>
#endif


std::queue<WebhookDispatcher::t_PendingWebhook>  WebhookDispatcher::queue_s;
std::mutex                                        WebhookDispatcher::queueMutex_s;
std::condition_variable                           WebhookDispatcher::queueCv_s;
string                                            WebhookDispatcher::secret_s;
int                                               WebhookDispatcher::maxRetries_s   = 3;
int                                               WebhookDispatcher::timeoutSeconds_s = 10;
std::atomic<bool>                                 WebhookDispatcher::running_s{false};
std::thread                                       WebhookDispatcher::workerThread_s;



void
WebhookDispatcher::initialize ()
{
    if (running_s.load())
        return;

    // Load config values
    string secretStr;
    if (Configuration::getValue("Webhook Secret"s, secretStr))
        secret_s = secretStr;

    string maxRetriesStr;
    if (Configuration::getValue("Webhook Max Retries"s, maxRetriesStr)) {
        auto parsed = parseInt(maxRetriesStr);
        if (parsed && *parsed >= 0 && *parsed <= 20)
            maxRetries_s = *parsed;
    }

    string timeoutStr;
    if (Configuration::getValue("Webhook Timeout Seconds"s, timeoutStr)) {
        auto parsed = parseInt(timeoutStr);
        if (parsed && *parsed >= 1 && *parsed <= 120)
            timeoutSeconds_s = *parsed;
    }

    running_s.store(true);
    workerThread_s = std::thread(workerLoop);

    Log::Info("WebhookDispatcher initialized (maxRetries="s +
              std::to_string(maxRetries_s) + ", timeout="s +
              std::to_string(timeoutSeconds_s) + "s)"s);
}



void
WebhookDispatcher::shutdown ()
{
    if (!running_s.load())
        return;

    running_s.store(false);
    queueCv_s.notify_all();

    if (workerThread_s.joinable())
        workerThread_s.join();

    // Drain remaining queue
    std::lock_guard<std::mutex> lock(queueMutex_s);
    while (!queue_s.empty())
        queue_s.pop();

    Log::Info("WebhookDispatcher shut down."s);
}



void
WebhookDispatcher::dispatch (const string & callbackUrl,
                             const string & jsonPayload,
                             const string & eventType)
{
    t_PendingWebhook webhook;
    webhook.callbackUrl = callbackUrl;
    webhook.jsonPayload = jsonPayload;
    webhook.eventType   = eventType;
    webhook.retryCount  = 0;
    webhook.nextAttempt = std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(queueMutex_s);
        queue_s.push(std::move(webhook));
    }

    queueCv_s.notify_one();
}



void
WebhookDispatcher::setSecret (const string & secret)
{
    secret_s = secret;
}



void
WebhookDispatcher::setMaxRetries (int maxRetries)
{
    if (maxRetries >= 0 && maxRetries <= 20)
        maxRetries_s = maxRetries;
}



void
WebhookDispatcher::setTimeoutSeconds (int timeout)
{
    if (timeout >= 1 && timeout <= 120)
        timeoutSeconds_s = timeout;
}



string
WebhookDispatcher::computeHmacSignature (const string & payload)
{
    if (secret_s.empty())
        return {};

#ifdef ALPINE_TLS_ENABLED
    unsigned char  hmacResult[EVP_MAX_MD_SIZE];
    unsigned int   hmacLen = 0;

    auto * result = HMAC(EVP_sha256(),
                         secret_s.data(),
                         static_cast<int>(secret_s.size()),
                         reinterpret_cast<const unsigned char *>(payload.data()),
                         payload.size(),
                         hmacResult,
                         &hmacLen);

    if (!result || hmacLen == 0)
        return {};

    // Convert to hex string
    static const char hexChars[] = "0123456789abcdef";
    string hexStr;
    hexStr.reserve(hmacLen * 2);
    for (unsigned int i = 0; i < hmacLen; ++i) {
        hexStr += hexChars[(hmacResult[i] >> 4) & 0x0F];
        hexStr += hexChars[hmacResult[i] & 0x0F];
    }

    return "sha256="s + hexStr;
#else
    return {};
#endif
}



bool
WebhookDispatcher::parseUrl (const string & url, t_UrlParts & parts)
{
    // Parse: scheme://host[:port][/path]
    string remaining = url;

    // Extract scheme
    auto schemeEnd = remaining.find("://"s);
    if (schemeEnd == string::npos)
        return false;

    string scheme = remaining.substr(0, schemeEnd);
    remaining = remaining.substr(schemeEnd + 3);

    if (scheme == "https"s)
        parts.useTls = true;
    else if (scheme == "http"s)
        parts.useTls = false;
    else
        return false;

    // Extract host[:port] and path
    auto pathStart = remaining.find('/');
    string hostPort;

    if (pathStart != string::npos) {
        hostPort = remaining.substr(0, pathStart);
        parts.path = remaining.substr(pathStart);
    } else {
        hostPort = remaining;
        parts.path = "/"s;
    }

    // Split host and port
    auto colonPos = hostPort.rfind(':');
    if (colonPos != string::npos && colonPos > 0) {
        // Check for IPv6 bracket notation
        auto bracketClose = hostPort.rfind(']');
        if (bracketClose == string::npos || colonPos > bracketClose) {
            parts.host = hostPort.substr(0, colonPos);
            parts.port = hostPort.substr(colonPos + 1);
        } else {
            parts.host = hostPort;
            parts.port = parts.useTls ? "443"s : "80"s;
        }
    } else {
        parts.host = hostPort;
        parts.port = parts.useTls ? "443"s : "80"s;
    }

    return !parts.host.empty();
}



void
WebhookDispatcher::workerLoop ()
{
    while (running_s.load()) {
        t_PendingWebhook  webhook;
        bool              haveWork = false;

        {
            std::unique_lock<std::mutex> lock(queueMutex_s);
            queueCv_s.wait(lock, [] {
                return !running_s.load() || !queue_s.empty();
            });

            if (!running_s.load() && queue_s.empty())
                break;

            if (!queue_s.empty()) {
                webhook = std::move(queue_s.front());
                queue_s.pop();
                haveWork = true;
            }
        }

        if (!haveWork)
            continue;

        // Wait until the scheduled attempt time (for retries with backoff)
        auto now = std::chrono::steady_clock::now();
        if (webhook.nextAttempt > now) {
            std::this_thread::sleep_until(webhook.nextAttempt);
            if (!running_s.load())
                break;
        }

        bool success = deliverWebhook(webhook);

        if (!success && webhook.retryCount < maxRetries_s) {
            webhook.retryCount++;

            // Exponential backoff: 1s, 5s, 30s
            static constexpr int backoffSeconds[] = {1, 5, 30};
            int backoffIndex = std::min(webhook.retryCount - 1, 2);
            webhook.nextAttempt = std::chrono::steady_clock::now() +
                                  std::chrono::seconds(backoffSeconds[backoffIndex]);

            Log::Debug("Webhook delivery failed, scheduling retry "s +
                       std::to_string(webhook.retryCount) + "/"s +
                       std::to_string(maxRetries_s) + " for "s +
                       webhook.callbackUrl);

            std::lock_guard<std::mutex> lock(queueMutex_s);
            queue_s.push(std::move(webhook));
            queueCv_s.notify_one();
        }
        else if (!success) {
            Log::Error("Webhook delivery failed after "s +
                       std::to_string(maxRetries_s) +
                       " retries for "s + webhook.callbackUrl);
        }
    }
}



bool
WebhookDispatcher::deliverWebhook (const t_PendingWebhook & webhook)
{
    t_UrlParts urlParts;
    if (!parseUrl(webhook.callbackUrl, urlParts)) {
        Log::Error("Webhook: invalid callback URL: "s + webhook.callbackUrl);
        return false;
    }

    try {
        asio::io_context ioContext;

        asio::ip::tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve(urlParts.host, urlParts.port);

        asio::ip::tcp::socket socket(ioContext);
        asio::connect(socket, endpoints);

        // Build HTTP POST request
        string requestStr;
        requestStr += "POST "s + urlParts.path + " HTTP/1.1\r\n"s;
        requestStr += "Host: "s + urlParts.host + "\r\n"s;
        requestStr += "Content-Type: application/json\r\n"s;
        requestStr += "Content-Length: "s + std::to_string(webhook.jsonPayload.size()) + "\r\n"s;
        requestStr += "X-Alpine-Event: "s + webhook.eventType + "\r\n"s;
        requestStr += "Connection: close\r\n"s;

        // Add HMAC signature if secret is configured
        string signature = computeHmacSignature(webhook.jsonPayload);
        if (!signature.empty())
            requestStr += "X-Alpine-Signature: "s + signature + "\r\n"s;

        requestStr += "\r\n"s;
        requestStr += webhook.jsonPayload;

        // Write with timeout using io_context.run_for
        asio::error_code writeEc;
        asio::async_write(socket,
                          asio::buffer(requestStr),
                          [&writeEc](const asio::error_code & ec, std::size_t) {
                              writeEc = ec;
                          });

        ioContext.run_for(std::chrono::seconds(timeoutSeconds_s));

        if (writeEc) {
            Log::Error("Webhook write failed: "s + writeEc.message());
            return false;
        }

        // Read response status line
        asio::streambuf responseBuf;
        asio::error_code readEc;
        asio::async_read_until(socket, responseBuf, "\r\n"s,
                               [&readEc](const asio::error_code & ec, std::size_t) {
                                   readEc = ec;
                               });

        ioContext.restart();
        ioContext.run_for(std::chrono::seconds(timeoutSeconds_s));

        if (readEc) {
            Log::Error("Webhook read failed: "s + readEc.message());
            return false;
        }

        // Parse HTTP status code from response
        std::istream responseStream(&responseBuf);
        string httpVersion;
        int statusCode = 0;
        responseStream >> httpVersion >> statusCode;

        if (statusCode >= 200 && statusCode < 300) {
            Log::Debug("Webhook delivered successfully to "s +
                       webhook.callbackUrl + " (HTTP "s +
                       std::to_string(statusCode) + ")"s);
            return true;
        }

        Log::Error("Webhook delivery got HTTP "s +
                   std::to_string(statusCode) + " from "s +
                   webhook.callbackUrl);
        return false;

    } catch (const std::exception & e) {
        Log::Error("Webhook delivery exception: "s + e.what());
        return false;
    }
}
