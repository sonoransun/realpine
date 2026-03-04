/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpResponse.h>
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
    headers_["Access-Control-Allow-Methods"] = "GET, POST, DELETE, OPTIONS";
    headers_["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    headers_["X-Content-Type-Options"]       = "nosniff";
    headers_["X-Frame-Options"]              = "DENY";
    headers_["Connection"]                   = "close";
}


void
HttpResponse::setCorsOrigin (const string & origin)
{
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


string
HttpResponse::build ()
{
    string result;

    // Status line
    char statusLine[64];
    snprintf(statusLine, sizeof(statusLine), "HTTP/1.1 %d ", statusCode_);
    result += statusLine;
    result += statusText_;
    result += "\r\n";

    // Content-Length header
    char contentLength[64];
    snprintf(contentLength, sizeof(contentLength), "%lu", (ulong)body_.length());
    headers_["Content-Length"] = contentLength;

    // Headers
    for (const auto& header : headers_)
    {
        result += header.first;
        result += ": ";
        result += header.second;
        result += "\r\n";
    }

    // Blank line
    result += "\r\n";

    // Body
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


HttpResponse
HttpResponse::notFound ()
{
    HttpResponse resp(404, "Not Found");
    resp.setJsonBody("{\"error\":\"Not Found\"}");
    return resp;
}


HttpResponse
HttpResponse::badRequest (const string & message)
{
    HttpResponse resp(400, "Bad Request");
    resp.setJsonBody("{\"error\":\"" + escapeJson(message) + "\"}");
    return resp;
}


HttpResponse
HttpResponse::methodNotAllowed ()
{
    HttpResponse resp(405, "Method Not Allowed");
    resp.setJsonBody("{\"error\":\"Method Not Allowed\"}");
    return resp;
}


HttpResponse
HttpResponse::serverError (const string & message)
{
    HttpResponse resp(500, "Internal Server Error");
    resp.setJsonBody("{\"error\":\"" + escapeJson(message) + "\"}");
    return resp;
}
