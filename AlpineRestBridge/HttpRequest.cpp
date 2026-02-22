/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpRequest.h>
#include <cctype>


// Ctor defaulted in header

// Dtor defaulted in header


bool
HttpRequest::parse (const byte *  data,
                    ulong         dataLength,
                    HttpRequest & request)
{
    if (!data || dataLength == 0)
        return false;

    string raw((const char *)data, dataLength);

    // Find end of request line
    ulong lineEnd = raw.find("\r\n");
    if (lineEnd == string::npos)
        return false;

    string requestLine = raw.substr(0, lineEnd);

    // Parse method
    ulong spacePos = requestLine.find(' ');
    if (spacePos == string::npos)
        return false;

    request.method = requestLine.substr(0, spacePos);

    // Parse path
    ulong pathStart = spacePos + 1;
    ulong pathEnd = requestLine.find(' ', pathStart);
    if (pathEnd == string::npos)
        return false;

    request.path = requestLine.substr(pathStart, pathEnd - pathStart);

    // Parse headers
    ulong pos = lineEnd + 2;  // skip past \r\n of request line

    while (pos < raw.length())
    {
        ulong headerEnd = raw.find("\r\n", pos);
        if (headerEnd == string::npos)
            break;

        // Empty line marks end of headers
        if (headerEnd == pos)
        {
            pos = headerEnd + 2;
            break;
        }

        string headerLine = raw.substr(pos, headerEnd - pos);

        ulong colonPos = headerLine.find(':');
        if (colonPos != string::npos)
        {
            string name = headerLine.substr(0, colonPos);
            string val  = headerLine.substr(colonPos + 1);

            // Lowercase the header name
            for (ulong i = 0; i < name.length(); ++i)
                name[i] = tolower(name[i]);

            // Trim leading whitespace from value
            ulong valStart = 0;
            while (valStart < val.length() && val[valStart] == ' ')
                ++valStart;

            if (valStart > 0)
                val = val.substr(valStart);

            request.headers[name] = val;
        }

        pos = headerEnd + 2;
    }

    // Body is everything after the blank line
    if (pos < raw.length())
        request.body = raw.substr(pos);

    return true;
}
