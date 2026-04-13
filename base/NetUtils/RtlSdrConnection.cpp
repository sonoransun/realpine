/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <RtlSdrConnection.h>
#include <SdrDemodulator.h>

#include <cstring>


#ifdef __linux__
#include <unistd.h>

#ifdef ALPINE_RTLSDR_ENABLED
#include <rtl-sdr.h>
#endif
#endif


#ifdef __linux__


RtlSdrConnection::RtlSdrConnection(uint centerFreqHz,
                                   uint sampleRate,
                                   int gainTenths,
                                   std::unique_ptr<SdrDemodulator> demodulator)
    : rtlDev_(nullptr),
      running_(false),
      demodulator_(std::move(demodulator)),
      centerFreqHz_(centerFreqHz),
      sampleRate_(sampleRate),
      gainTenths_(gainTenths)
{
    pipeFds_[0] = -1;
    pipeFds_[1] = -1;
}


RtlSdrConnection::~RtlSdrConnection()
{
    close();
}


bool
RtlSdrConnection::create(ulong ipAddress, ushort port)
{
#ifndef ALPINE_RTLSDR_ENABLED
    Log::Error("RtlSdrConnection: RTL-SDR support not compiled in (need -DALPINE_ENABLE_RTLSDR=ON).");
    return false;
#else
    if (running_.load()) {
        close();
    }

    if (!demodulator_) {
        Log::Error("RtlSdrConnection: No demodulator provided.");
        return false;
    }

    // Create self-pipe for PollSet integration
    if (pipe2(pipeFds_, O_NONBLOCK | O_CLOEXEC) < 0) {
        Log::Error("RtlSdrConnection: pipe2() failed.");
        return false;
    }

    // Open RTL-SDR device (device index 0)
    int result = rtlsdr_open(reinterpret_cast<rtlsdr_dev_t **>(&rtlDev_), 0);

    if (result < 0) {
        Log::Error("RtlSdrConnection: rtlsdr_open failed (result="s + std::to_string(result) + ").");
        ::close(pipeFds_[0]);
        ::close(pipeFds_[1]);
        pipeFds_[0] = -1;
        pipeFds_[1] = -1;
        return false;
    }

    auto * dev = static_cast<rtlsdr_dev_t *>(rtlDev_);

    // Configure device
    result = rtlsdr_set_sample_rate(dev, sampleRate_);

    if (result < 0) {
        Log::Error("RtlSdrConnection: Failed to set sample rate.");
    }

    result = rtlsdr_set_center_freq(dev, centerFreqHz_);

    if (result < 0) {
        Log::Error("RtlSdrConnection: Failed to set center frequency.");
    }

    if (gainTenths_ == 0) {
        rtlsdr_set_tuner_gain_mode(dev, 0);  // auto gain
    } else {
        rtlsdr_set_tuner_gain_mode(dev, 1);  // manual
        rtlsdr_set_tuner_gain(dev, gainTenths_);
    }

    rtlsdr_reset_buffer(dev);

    // Initialize demodulator
    if (!demodulator_->initialize(sampleRate_)) {
        Log::Error("RtlSdrConnection: Demodulator initialization failed.");
        rtlsdr_close(dev);
        rtlDev_ = nullptr;
        ::close(pipeFds_[0]);
        ::close(pipeFds_[1]);
        pipeFds_[0] = -1;
        pipeFds_[1] = -1;
        return false;
    }

    // Start async reader thread
    running_.store(true);
    asyncThread_ = std::thread(&RtlSdrConnection::asyncReaderThread, this);

    Log::Info("RtlSdrConnection: Created (freq="s + std::to_string(centerFreqHz_) +
              " Hz, rate=" + std::to_string(sampleRate_) + ", gain=" + std::to_string(gainTenths_) + ")");

    return true;
#endif  // ALPINE_RTLSDR_ENABLED
}


void
RtlSdrConnection::close()
{
    if (!running_.load() && pipeFds_[0] < 0) {
        return;
    }

    running_.store(false);

#ifdef ALPINE_RTLSDR_ENABLED
    if (rtlDev_) {
        rtlsdr_cancel_async(static_cast<rtlsdr_dev_t *>(rtlDev_));
    }
#endif

    if (asyncThread_.joinable()) {
        asyncThread_.join();
    }

#ifdef ALPINE_RTLSDR_ENABLED
    if (rtlDev_) {
        rtlsdr_close(static_cast<rtlsdr_dev_t *>(rtlDev_));
        rtlDev_ = nullptr;
    }
#endif

    if (pipeFds_[0] >= 0) {
        ::close(pipeFds_[0]);
        pipeFds_[0] = -1;
    }

    if (pipeFds_[1] >= 0) {
        ::close(pipeFds_[1]);
        pipeFds_[1] = -1;
    }

    {
        std::lock_guard lock(queueMutex_);
        packetQueue_.clear();
    }
}


int
RtlSdrConnection::getFd()
{
    return pipeFds_[0];
}


bool
RtlSdrConnection::sendData(const ulong destIpAddress,
                           const ushort destPort,
                           const byte * dataBuffer,
                           const uint dataLength)
{
    // RTL-SDR is receive-only
    return false;
}


bool
RtlSdrConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    if (pipeFds_[0] < 0) {
        return false;
    }

    // Drain the wakeup byte from the pipe
    byte dummy;
    [[maybe_unused]] auto rd = read(pipeFds_[0], &dummy, 1);

    std::lock_guard lock(queueMutex_);

    if (packetQueue_.empty()) {
        return false;
    }

    auto & entry = packetQueue_.front();

    // The payload contains the addressed format:
    // [destIp:4][destPort:2][srcIp:4][srcPort:2][data:N]
    if (entry.data.size() < 12) {
        packetQueue_.pop_front();
        return false;
    }

    // Extract source addressing from the payload prefix
    uint off = 4 + 2;  // skip dest
    memcpy(&sourceIpAddress, entry.data.data() + off, 4);
    off += 4;
    memcpy(&sourcePort, entry.data.data() + off, 2);
    off += 2;

    uint actualDataLen = static_cast<uint>(entry.data.size()) - off;

    if (actualDataLen > bufferLength) {
        actualDataLen = bufferLength;
    }

    memcpy(dataBuffer, entry.data.data() + off, actualDataLen);
    dataLength = actualDataLen;

    packetQueue_.pop_front();

    return true;
}


#ifdef ALPINE_RTLSDR_ENABLED
void
RtlSdrConnection::rtlsdrCallback(unsigned char * buf, uint32_t len, void * ctx)
{
    auto * self = static_cast<RtlSdrConnection *>(ctx);

    if (!self->running_.load()) {
        return;
    }

    self->processIqSamples(reinterpret_cast<const int8_t *>(buf), len);
}
#endif


void
RtlSdrConnection::processIqSamples(const int8_t * samples, uint numSamples)
{
    if (!demodulator_) {
        return;
    }

    vector<vector<byte>> packets;

    if (!demodulator_->demodulate(samples, numSamples, packets)) {
        return;
    }

    for (auto & packet : packets) {
        std::lock_guard lock(queueMutex_);

        if (packetQueue_.size() >= MAX_QUEUE_SIZE) {
            // Drop oldest to prevent unbounded growth
            packetQueue_.pop_front();
        }

        PacketEntry entry;
        entry.data = std::move(packet);
        // Source IP/port are embedded in the addressed payload;
        // receiveData() will extract them.
        entry.sourceIp = 0;
        entry.sourcePort = 0;
        packetQueue_.push_back(std::move(entry));

        // Wake PollSet
        byte wakeup = 1;
        [[maybe_unused]] auto wr = write(pipeFds_[1], &wakeup, 1);
    }
}


void
RtlSdrConnection::asyncReaderThread()
{
#ifdef ALPINE_RTLSDR_ENABLED
    if (!rtlDev_) {
        return;
    }

    auto * dev = static_cast<rtlsdr_dev_t *>(rtlDev_);

    // Block size: 16K IQ samples (32K bytes) — a common default
    static constexpr uint32_t BLOCK_SIZE = 32 * 1024;
    static constexpr int NUM_BUFFERS = 8;

    // rtlsdr_read_async blocks until rtlsdr_cancel_async is called
    int result = rtlsdr_read_async(dev, rtlsdrCallback, this, NUM_BUFFERS, BLOCK_SIZE);

    if (result < 0 && running_.load()) {
        Log::Error("RtlSdrConnection: rtlsdr_read_async returned error: "s + std::to_string(result));
    }
#endif
}


#else  // Non-Linux stubs


RtlSdrConnection::RtlSdrConnection(uint centerFreqHz,
                                   uint sampleRate,
                                   int gainTenths,
                                   std::unique_ptr<SdrDemodulator> demodulator)
    : demodulator_(std::move(demodulator)),
      centerFreqHz_(centerFreqHz),
      sampleRate_(sampleRate),
      gainTenths_(gainTenths)
{}


RtlSdrConnection::~RtlSdrConnection() {}


bool
RtlSdrConnection::create(ulong ipAddress, ushort port)
{
    Log::Error("RtlSdrConnection: RTL-SDR transport is only supported on Linux.");
    return false;
}


void
RtlSdrConnection::close()
{}


int
RtlSdrConnection::getFd()
{
    return -1;
}


bool
RtlSdrConnection::sendData(const ulong destIpAddress,
                           const ushort destPort,
                           const byte * dataBuffer,
                           const uint dataLength)
{
    return false;
}


bool
RtlSdrConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    return false;
}


#endif
