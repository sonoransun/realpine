/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unordered_map>


class HttpResponse
{
  public:

    HttpResponse (int            statusCode,
                  const string & statusText);

    ~HttpResponse () = default;


    void  setHeader (const string & name,
                     const string & value);

    void  setBody (const string & body);

    void  setJsonBody (const string & json);

    string  build ();


    static HttpResponse  ok (const string & json);

    static HttpResponse  accepted (const string & json);

    static HttpResponse  notFound ();

    static HttpResponse  badRequest (const string & message);

    static HttpResponse  methodNotAllowed ();

    static HttpResponse  serverError (const string & message);

    static void  setCorsOrigin (const string & origin);


  private:

    void  addCorsHeaders ();

    int     statusCode_;
    string  statusText_;
    string  body_;

    std::unordered_map<string, string>  headers_;

    static string  corsOrigin_s;

};
