///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <HttpResponse.h>
#include <cstdio>


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
    headers_["Access-Control-Allow-Origin"]  = "*";
    headers_["Access-Control-Allow-Methods"] = "GET, POST, DELETE, OPTIONS";
    headers_["Access-Control-Allow-Headers"] = "Content-Type";
    headers_["Connection"]                   = "close";
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
    resp.setJsonBody("{\"error\":\"" + message + "\"}");
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
    resp.setJsonBody("{\"error\":\"" + message + "\"}");
    return resp;
}
