/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#ifdef ALPINE_UPNP_ENABLED

#include <Log.h>
#include <UpnpPortMapper.h>

#include <miniupnpc.h>
#include <upnpcommands.h>
#include <upnperrors.h>

#include <cstring>


ReadWriteSem UpnpPortMapper::semaphore_;
std::vector<UpnpPortMapper::PortMapping> UpnpPortMapper::activeMappings_;
bool UpnpPortMapper::available_ = false;
UpnpPortMapper::RenewalThread * UpnpPortMapper::renewalThread_ = nullptr;
void * UpnpPortMapper::devList_ = nullptr;
char UpnpPortMapper::lanAddr_[64] = {};
void * UpnpPortMapper::urls_ = nullptr;
void * UpnpPortMapper::igdData_ = nullptr;


bool
UpnpPortMapper::initialize()
{
    semaphore_.acquireWrite();

    int error = 0;
    auto * devList = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2, &error);

    if (!devList) {
        Log::Info("UpnpPortMapper: No UPnP IGD devices found.");
        semaphore_.releaseWrite();
        return false;
    }

    devList_ = devList;

    auto * urls = new UPNPUrls;
    auto * igdData = new IGDdatas;
    std::memset(urls, 0, sizeof(UPNPUrls));
    std::memset(igdData, 0, sizeof(IGDdatas));

    int status = UPNP_GetValidIGD(devList, urls, igdData, lanAddr_, sizeof(lanAddr_));

    if (status != 1) {
        Log::Info("UpnpPortMapper: No valid IGD found (status="s + std::to_string(status) + ").");
        delete urls;
        delete igdData;
        freeUPNPDevlist(devList);
        devList_ = nullptr;
        semaphore_.releaseWrite();
        return false;
    }

    urls_ = urls;
    igdData_ = igdData;
    available_ = true;

    Log::Info("UpnpPortMapper: Found IGD, local address: "s + lanAddr_);

    // Start lease renewal thread
    renewalThread_ = new RenewalThread();
    renewalThread_->create();

    semaphore_.releaseWrite();
    return true;
}


bool
UpnpPortMapper::addMapping(ushort externalPort,
                           ushort internalPort,
                           const string & protocol,
                           const string & description)
{
    semaphore_.acquireWrite();

    if (!available_) {
        semaphore_.releaseWrite();
        return false;
    }

    auto * urls = static_cast<UPNPUrls *>(urls_);
    auto * igdData = static_cast<IGDdatas *>(igdData_);

    string extPortStr = std::to_string(externalPort);
    string intPortStr = std::to_string(internalPort);
    string leaseStr = std::to_string(LEASE_DURATION_SEC);

    int result = UPNP_AddPortMapping(urls->controlURL,
                                     igdData->first.servicetype,
                                     extPortStr.c_str(),
                                     intPortStr.c_str(),
                                     lanAddr_,
                                     description.c_str(),
                                     protocol.c_str(),
                                     nullptr,  // remote host (any)
                                     leaseStr.c_str());

    if (result != UPNPCOMMAND_SUCCESS) {
        Log::Error("UpnpPortMapper: Failed to add port mapping "s + extPortStr + " -> " + intPortStr + " " + protocol +
                   " (error " + std::to_string(result) + ")");
        semaphore_.releaseWrite();
        return false;
    }

    PortMapping mapping;
    mapping.externalPort = externalPort;
    mapping.internalPort = internalPort;
    mapping.protocol = protocol;
    mapping.description = description;
    activeMappings_.push_back(mapping);

    Log::Info("UpnpPortMapper: Added mapping "s + extPortStr + " -> " + intPortStr + " " + protocol + " (" +
              description + ")");

    semaphore_.releaseWrite();
    return true;
}


bool
UpnpPortMapper::removeMapping(ushort externalPort, const string & protocol)
{
    semaphore_.acquireWrite();

    if (!available_) {
        semaphore_.releaseWrite();
        return false;
    }

    auto * urls = static_cast<UPNPUrls *>(urls_);
    auto * igdData = static_cast<IGDdatas *>(igdData_);

    string extPortStr = std::to_string(externalPort);

    int result = UPNP_DeletePortMapping(urls->controlURL,
                                        igdData->first.servicetype,
                                        extPortStr.c_str(),
                                        protocol.c_str(),
                                        nullptr  // remote host
    );

    // Remove from tracking regardless of result
    auto it = activeMappings_.begin();
    while (it != activeMappings_.end()) {
        if (it->externalPort == externalPort && it->protocol == protocol)
            it = activeMappings_.erase(it);
        else
            ++it;
    }

    semaphore_.releaseWrite();

    if (result != UPNPCOMMAND_SUCCESS) {
        Log::Debug("UpnpPortMapper: Failed to remove mapping "s + extPortStr + " " + protocol + " (error " +
                   std::to_string(result) + ")");
        return false;
    }

    Log::Info("UpnpPortMapper: Removed mapping "s + extPortStr + " " + protocol);
    return true;
}


string
UpnpPortMapper::getExternalIpAddress()
{
    semaphore_.acquireRead();

    if (!available_) {
        semaphore_.releaseRead();
        return {};
    }

    auto * urls = static_cast<UPNPUrls *>(urls_);
    auto * igdData = static_cast<IGDdatas *>(igdData_);

    char externalAddr[40] = {};
    int result = UPNP_GetExternalIPAddress(urls->controlURL, igdData->first.servicetype, externalAddr);

    semaphore_.releaseRead();

    if (result != UPNPCOMMAND_SUCCESS)
        return {};

    return string(externalAddr);
}


bool
UpnpPortMapper::isAvailable()
{
    semaphore_.acquireRead();
    bool avail = available_;
    semaphore_.releaseRead();
    return avail;
}


void
UpnpPortMapper::shutdown()
{
    semaphore_.acquireWrite();

    // Stop renewal thread
    if (renewalThread_) {
        renewalThread_->destroy();
        delete renewalThread_;
        renewalThread_ = nullptr;
    }

    if (available_) {
        auto * urls = static_cast<UPNPUrls *>(urls_);
        auto * igdData = static_cast<IGDdatas *>(igdData_);

        // Remove all active mappings
        for (const auto & mapping : activeMappings_) {
            string extPortStr = std::to_string(mapping.externalPort);

            UPNP_DeletePortMapping(
                urls->controlURL, igdData->first.servicetype, extPortStr.c_str(), mapping.protocol.c_str(), nullptr);

            Log::Debug("UpnpPortMapper: Removed mapping "s + extPortStr + " " + mapping.protocol + " during shutdown");
        }

        activeMappings_.clear();

        FreeUPNPUrls(urls);
        delete urls;
        delete igdData;
        urls_ = nullptr;
        igdData_ = nullptr;
    }

    if (devList_) {
        freeUPNPDevlist(static_cast<UPNPDev *>(devList_));
        devList_ = nullptr;
    }

    available_ = false;

    semaphore_.releaseWrite();

    Log::Info("UpnpPortMapper: Shutdown complete.");
}


void
UpnpPortMapper::renewLeases()
{
    semaphore_.acquireRead();

    if (!available_ || activeMappings_.empty()) {
        semaphore_.releaseRead();
        return;
    }

    auto * urls = static_cast<UPNPUrls *>(urls_);
    auto * igdData = static_cast<IGDdatas *>(igdData_);
    string leaseStr = std::to_string(LEASE_DURATION_SEC);

    // Copy mappings to avoid holding lock during network calls
    auto mappings = activeMappings_;
    semaphore_.releaseRead();

    for (const auto & mapping : mappings) {
        string extPortStr = std::to_string(mapping.externalPort);
        string intPortStr = std::to_string(mapping.internalPort);

        int result = UPNP_AddPortMapping(urls->controlURL,
                                         igdData->first.servicetype,
                                         extPortStr.c_str(),
                                         intPortStr.c_str(),
                                         lanAddr_,
                                         mapping.description.c_str(),
                                         mapping.protocol.c_str(),
                                         nullptr,
                                         leaseStr.c_str());

        if (result != UPNPCOMMAND_SUCCESS) {
            Log::Error("UpnpPortMapper: Lease renewal failed for "s + extPortStr + " " + mapping.protocol + " (error " +
                       std::to_string(result) + ")");
        }
    }

    Log::Debug("UpnpPortMapper: Lease renewal completed for "s + std::to_string(mappings.size()) + " mappings");
}


void
UpnpPortMapper::RenewalThread::threadMain()
{
    Log::Debug("UpnpPortMapper: Renewal thread started.");

    while (isActive()) {
        // Sleep in small increments so we can check isActive()
        for (int i = 0; i < RENEWAL_INTERVAL_SEC && isActive(); ++i)
            std::this_thread::sleep_for(std::chrono::seconds(1));

        if (isActive())
            renewLeases();
    }

    Log::Debug("UpnpPortMapper: Renewal thread exiting.");
}


#endif  // ALPINE_UPNP_ENABLED
