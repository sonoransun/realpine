/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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
