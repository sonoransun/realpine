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


#pragma once
#include <Common.h>
#include <ContentStore.h>


class DlnaHandler
{
  public:

    // UPnP device/service descriptions
    //
    static string  deviceDescription (const string & uuid, const string & baseUrl);
    static string  cdsServiceDescription ();
    static string  cmsServiceDescription ();

    // SOAP action dispatch
    //
    static string  handleCdsAction (const string & soapBody,
                                    const string & soapAction,
                                    ContentStore & store,
                                    const string & baseUrl,
                                    bool transcodeEnabled);

    static string  handleCmsAction (const string & soapBody,
                                    const string & soapAction);


  private:

    // CDS actions
    //
    static string  actionBrowse (const string & soapBody,
                                 ContentStore & store,
                                 const string & baseUrl,
                                 bool transcodeEnabled);

    // CMS actions
    //
    static string  actionGetProtocolInfo ();

    // DIDL-Lite helpers
    //
    static string  buildDidlItem (const ContentStore::MediaItem & item,
                                  const string & baseUrl,
                                  bool transcodeEnabled);

    static string  buildDidlContainer (ulong childCount);

    static string  wrapSoapResponse (const string & action,
                                     const string & svcType,
                                     const string & body);

    static string  extractXmlValue (const string & xml, const string & tag);

};
