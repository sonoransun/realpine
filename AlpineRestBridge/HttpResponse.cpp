/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpResponse.h>
#include <StringUtils.h>
#include <Log.h>
#include <cstdio>


string HttpResponse::corsOrigin_s;


static string escapeJson(const string& s) {
    string result;
    result.reserve(s.length());
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}


HttpResponse::HttpResponse (int            statusCode,
                            const string & statusText)
    : statusCode_(statusCode),
      statusText_(statusText)
{
    addCorsHeaders();
}


// Dtor defaulted in header


void
HttpResponse::addCorsHeaders ()
{
    if (!corsOrigin_s.empty())
        headers_["Access-Control-Allow-Origin"] = corsOrigin_s;

    if (!corsOrigin_s.empty() && corsOrigin_s != "*"s) {
        // Specific domain: allow DELETE and enable credentials
        headers_["Access-Control-Allow-Methods"]     = "GET, POST, DELETE, OPTIONS";
        headers_["Access-Control-Allow-Credentials"] = "true";
    } else {
        // Wildcard or empty origin: restrict to safe methods, no DELETE
        headers_["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
    }

    headers_["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    headers_["X-Content-Type-Options"]       = "nosniff";
    headers_["X-Frame-Options"]              = "DENY";
    headers_["Content-Security-Policy"]      = "default-src 'none'";
    headers_["Referrer-Policy"]              = "no-referrer";
    headers_["Cache-Control"]                = "no-store";
#ifdef ALPINE_TLS_ENABLED
    headers_["Strict-Transport-Security"]    = "max-age=31536000; includeSubDomains";
#endif
    headers_["Connection"]                   = "keep-alive";
}


void
HttpResponse::setCorsOrigin (const string & origin)
{
    if (origin != "*"s &&
        !origin.starts_with("http://"s) &&
        !origin.starts_with("https://"s)) {
        Log::Error("HttpResponse: rejecting invalid CORS origin: "s + StringUtils::sanitizeForLog(origin));
        return;
    }
    corsOrigin_s = origin;
}


void
HttpResponse::setHeader (const string & name,
                         const string & value)
{
    headers_[name] = value;
}


void
HttpResponse::setBody (const string & body)
{
    body_ = body;
}


void
HttpResponse::setJsonBody (const string & json)
{
    headers_["Content-Type"] = "application/json";
    body_ = json;
}


void
HttpResponse::setRequestId (const string & requestId)
{
    requestId_ = requestId;
    headers_["X-Request-ID"] = requestId;
}


void
HttpResponse::setConnectionClose ()
{
    headers_["Connection"] = "close";
    headers_.erase("Keep-Alive");
}


void
HttpResponse::setKeepAliveParams (int timeout, int maxRequests)
{
    headers_["Keep-Alive"] = "timeout="s + std::to_string(timeout)
                             + ", max="s + std::to_string(maxRequests);
}


string
HttpResponse::build ()
{
    // Content-Length header (compute before size estimation)
    char contentLength[64];
    snprintf(contentLength, sizeof(contentLength), "%lu", (ulong)body_.length());
    headers_["Content-Length"] = contentLength;

    // Pre-calculate total size to avoid reallocations
    // Status line: "HTTP/1.1 " + code(3) + " " + statusText + "\r\n"
    size_t totalSize = 9 + 3 + 1 + statusText_.size() + 2;
    for (const auto & header : headers_) {
        totalSize += header.first.size() + 2 + header.second.size() + 2;
    }
    totalSize += 2;  // blank line
    totalSize += body_.size();

    string result;
    result.reserve(totalSize + 32);

    // Status line
    char statusLine[64];
    snprintf(statusLine, sizeof(statusLine), "HTTP/1.1 %d ", statusCode_);
    result += statusLine;
    result += statusText_;
    result += "\r\n";

    // Headers
    for (const auto & header : headers_) {
        result += header.first;
        result += ": ";
        result += header.second;
        result += "\r\n";
    }

    // Blank line + body
    result += "\r\n";
    result += body_;

    return result;
}


HttpResponse
HttpResponse::ok (const string & json)
{
    HttpResponse resp(200, "OK");
    resp.setJsonBody(json);
    return resp;
}


HttpResponse
HttpResponse::accepted (const string & json)
{
    HttpResponse resp(202, "Accepted");
    resp.setJsonBody(json);
    return resp;
}


string
HttpResponse::buildErrorJson (const string & code,
                              const string & message,
                              const string & requestId)
{
    string json = "{\"error\":{\"code\":\""s + escapeJson(code) +
                  "\",\"message\":\""s + escapeJson(message) + "\""s;
    if (!requestId.empty())
        json += ",\"requestId\":\""s + escapeJson(requestId) + "\""s;
    json += "}}"s;
    return json;
}


HttpResponse
HttpResponse::notFound (const string & errorCode)
{
    HttpResponse resp(404, "Not Found");
    resp.setJsonBody(buildErrorJson(errorCode, "Not Found"));
    return resp;
}


HttpResponse
HttpResponse::badRequest (const string & message,
                          const string & errorCode)
{
    HttpResponse resp(400, "Bad Request");
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}


HttpResponse
HttpResponse::methodNotAllowed ()
{
    HttpResponse resp(405, "Method Not Allowed");
    resp.setJsonBody(buildErrorJson("METHOD_NOT_ALLOWED"s, "Method Not Allowed"s));
    return resp;
}


HttpResponse
HttpResponse::tooManyRequests (const string & message,
                               const string & errorCode)
{
    HttpResponse resp(429, "Too Many Requests");
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}


HttpResponse
HttpResponse::unauthorized (const string & message,
                            const string & errorCode)
{
    HttpResponse resp(401, "Unauthorized");
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}


HttpResponse
HttpResponse::serverError (const string & message,
                           const string & errorCode)
{
    HttpResponse resp(500, "Internal Server Error");
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}


HttpResponse
HttpResponse::serviceUnavailable (const string & message,
                                  const string & errorCode)
{
    HttpResponse resp(503, "Service Unavailable");
    resp.setHeader("Retry-After"s, "5"s);
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}


HttpResponse
HttpResponse::conflict (const string & message,
                        const string & errorCode)
{
    HttpResponse resp(409, "Conflict");
    resp.setJsonBody(buildErrorJson(errorCode, message));
    return resp;
}
