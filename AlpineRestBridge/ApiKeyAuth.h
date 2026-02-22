/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>

class ApiKeyAuth
{
  public:
    static void initialize();
    static bool validate(const HttpRequest& request, HttpResponse& response);
    static const string& getKey();

  private:
    static bool constantTimeCompare(const string& a, const string& b);
    static string apiKey_s;
    static bool initialized_s;
};
