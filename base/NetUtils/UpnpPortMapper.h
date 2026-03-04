/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>

#ifdef ALPINE_UPNP_ENABLED

#include <ReadWriteSem.h>
#include <AutoThread.h>
#include <vector>


class UpnpPortMapper
{
  public:

    static bool initialize ();

    static bool addMapping (ushort externalPort, ushort internalPort,
                            const string & protocol,
                            const string & description);

    static bool removeMapping (ushort externalPort, const string & protocol);

    static string getExternalIpAddress ();

    static bool isAvailable ();

    static void shutdown ();


  private:

    struct PortMapping {
        ushort      externalPort;
        ushort      internalPort;
        string      protocol;
        string      description;
    };

    class RenewalThread : public AutoThread
    {
      public:
        void threadMain () override;
    };

    static void renewLeases ();

    static ReadWriteSem                 semaphore_;
    static std::vector<PortMapping>     activeMappings_;
    static bool                         available_;
    static RenewalThread *              renewalThread_;

    // miniupnpc state (opaque pointers)
    static void *                       devList_;
    static char                         lanAddr_[64];
    static void *                       urls_;
    static void *                       igdData_;

    static constexpr int LEASE_DURATION_SEC = 3600;
    static constexpr int RENEWAL_INTERVAL_SEC = 3000;  // ~50 min
};


#else  // ALPINE_UPNP_ENABLED not defined


class UpnpPortMapper
{
  public:

    static bool initialize ()   { return false; }

    static bool addMapping (ushort, ushort, const string &, const string &)
                                { return false; }

    static bool removeMapping (ushort, const string &)
                                { return false; }

    static string getExternalIpAddress ()
                                { return {}; }

    static bool isAvailable ()  { return false; }

    static void shutdown ()     {}
};


#endif  // ALPINE_UPNP_ENABLED
