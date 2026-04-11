/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string_view>
#include <unordered_map>


class HttpResponse
{
  public:
    // Structured error codes
    static constexpr const char * EC_INVALID_PARAMETER = "INVALID_PARAMETER";
    static constexpr const char * EC_NOT_FOUND = "NOT_FOUND";
    static constexpr const char * EC_RATE_LIMITED = "RATE_LIMITED";
    static constexpr const char * EC_UNAUTHORIZED = "UNAUTHORIZED";
    static constexpr const char * EC_INTERNAL_ERROR = "INTERNAL_ERROR";
    static constexpr const char * EC_SERVICE_UNAVAILABLE = "SERVICE_UNAVAILABLE";
    static constexpr const char * EC_CONFLICT = "CONFLICT";


    HttpResponse(int statusCode, const string & statusText);

    ~HttpResponse() = default;


    void setHeader(const string & name, const string & value);

    void setBody(const string & body);

    void setJsonBody(const string & json);

    void setRequestId(const string & requestId);

    void setConnectionClose();

    void setKeepAliveParams(int timeout, int maxRequests);

    int
    statusCode() const
    {
        return statusCode_;
    }

    ulong
    bodySize() const
    {
        return body_.size();
    }

    void compressBody(const string & encoding);

    string build();


    static HttpResponse ok(const string & json);

    static HttpResponse accepted(const string & json);

    static HttpResponse notFound(const string & errorCode = EC_NOT_FOUND);

    static HttpResponse badRequest(const string & message, const string & errorCode = EC_INVALID_PARAMETER);

    static HttpResponse methodNotAllowed();

    static HttpResponse tooManyRequests(const string & message, const string & errorCode = EC_RATE_LIMITED);

    static HttpResponse unauthorized(const string & message = "Unauthorized"s,
                                     const string & errorCode = EC_UNAUTHORIZED);

    static HttpResponse serverError(const string & message, const string & errorCode = EC_INTERNAL_ERROR);

    static HttpResponse serviceUnavailable(const string & message = "Service Unavailable"s,
                                           const string & errorCode = EC_SERVICE_UNAVAILABLE);

    static HttpResponse conflict(const string & message, const string & errorCode = EC_CONFLICT);

    static void setCorsOrigin(const string & origin);


  private:
    static string buildErrorJson(const string & code, const string & message, const string & requestId = {});

    void addCorsHeaders();

    int statusCode_;
    string statusText_;
    string body_;
    string requestId_;

    std::unordered_map<string, string> headers_;

    static string corsOrigin_s;
};
