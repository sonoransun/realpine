/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once

//
// Platform.h — Cross-platform compatibility layer for POSIX/Win32.
//
// Include this header instead of raw POSIX headers in platform-dependent code.
// It provides unified access to sockets, poll, dynamic loading, time, process
// management, and file control across Linux, macOS, and Windows (MinGW-w64).
//


// ============================================================================
//  Platform detection
// ============================================================================

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
    #define ALPINE_PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define ALPINE_PLATFORM_LINUX   1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #define ALPINE_PLATFORM_DARWIN  1
    #if TARGET_OS_IOS
        #define ALPINE_PLATFORM_IOS 1
    #endif
#endif

#if defined(ALPINE_PLATFORM_LINUX) || defined(ALPINE_PLATFORM_DARWIN)
    #define ALPINE_PLATFORM_POSIX   1
#endif


// ============================================================================
//  Sockets
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    // POSIX socket compatibility
    using socklen_t  = int;
    using ssize_t    = long long;

    // On Windows, socket handles are SOCKET (unsigned), not int.
    // Most Alpine code uses int for FDs.  The INVALID_SOCKET sentinel is ~0,
    // but we keep the POSIX convention of -1 via a thin cast layer.
    inline int  alpine_close_socket(int fd) { return ::closesocket(static_cast<SOCKET>(fd)); }

    // Map POSIX errno values to Winsock equivalents
    #ifndef EAGAIN
    #define EAGAIN          WSAEWOULDBLOCK
    #endif
    #ifndef ECONNRESET
    #define ECONNRESET      WSAECONNRESET
    #endif
    #ifndef ECONNREFUSED
    #define ECONNREFUSED    WSAECONNREFUSED
    #endif
    #ifndef ENETUNREACH
    #define ENETUNREACH     WSAENETUNREACH
    #endif
    #ifndef EHOSTUNREACH
    #define EHOSTUNREACH    WSAEHOSTUNREACH
    #endif
    #ifndef EINPROGRESS
    #define EINPROGRESS     WSAEINPROGRESS
    #endif
    #ifndef EWOULDBLOCK
    #define EWOULDBLOCK     WSAEWOULDBLOCK
    #endif

    // errno → WSAGetLastError() for socket operations
    inline int alpine_socket_errno() { return WSAGetLastError(); }

    // Winsock must be initialized before any socket call.
    // Call this once at program startup.
    inline bool alpine_socket_init() {
        WSADATA wsa;
        return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
    }
    inline void alpine_socket_cleanup() {
        WSACleanup();
    }

#else  // POSIX

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>

    inline int  alpine_close_socket(int fd) { return ::close(fd); }
    inline int  alpine_socket_errno()       { return errno; }
    inline bool alpine_socket_init()        { return true; }
    inline void alpine_socket_cleanup()     {}

#endif


// ============================================================================
//  Poll
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    // WSAPoll is available on Windows Vista+ and has the same struct layout
    // as POSIX poll (WSAPOLLFD ≡ struct pollfd).
    #ifndef POLLIN
    #define POLLIN   0x0100
    #endif
    #ifndef POLLOUT
    #define POLLOUT  0x0010
    #endif
    #ifndef POLLERR
    #define POLLERR  0x0001
    #endif
    #ifndef POLLHUP
    #define POLLHUP  0x0002
    #endif
    #ifndef POLLNVAL
    #define POLLNVAL 0x0004
    #endif

    struct pollfd {
        SOCKET fd;
        short  events;
        short  revents;
    };

    inline int alpine_poll(struct pollfd* fds, unsigned long nfds, int timeout) {
        return WSAPoll(reinterpret_cast<WSAPOLLFD*>(fds), nfds, timeout);
    }

#else  // POSIX

    #include <sys/poll.h>

    inline int alpine_poll(struct pollfd* fds, unsigned long nfds, int timeout) {
        return ::poll(fds, static_cast<nfds_t>(nfds), timeout);
    }

#endif


// ============================================================================
//  File control / non-blocking
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    inline bool alpine_set_nonblocking(int fd) {
        u_long mode = 1;
        return ioctlsocket(static_cast<SOCKET>(fd), FIONBIO, &mode) == 0;
    }
    inline bool alpine_set_blocking(int fd) {
        u_long mode = 0;
        return ioctlsocket(static_cast<SOCKET>(fd), FIONBIO, &mode) == 0;
    }

#else  // POSIX

    #include <fcntl.h>

    inline bool alpine_set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) return false;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK) >= 0;
    }
    inline bool alpine_set_blocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) return false;
        return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) >= 0;
    }

#endif


// ============================================================================
//  Dynamic loading
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    // Map POSIX dl flags to Windows equivalents (no-ops on Windows)
    #define RTLD_NOW    0
    #define RTLD_LAZY   0
    #define RTLD_GLOBAL 0

    using alpine_dl_handle = HMODULE;

    inline alpine_dl_handle alpine_dlopen(const char* path, int /*flags*/) {
        return LoadLibraryA(path);
    }
    inline void* alpine_dlsym(alpine_dl_handle handle, const char* symbol) {
        return reinterpret_cast<void*>(GetProcAddress(handle, symbol));
    }
    inline int alpine_dlclose(alpine_dl_handle handle) {
        return FreeLibrary(handle) ? 0 : -1;
    }

#else  // POSIX

    #include <dlfcn.h>

    using alpine_dl_handle = void*;

    inline alpine_dl_handle alpine_dlopen(const char* path, int flags) {
        return dlopen(path, flags);
    }
    inline void* alpine_dlsym(alpine_dl_handle handle, const char* symbol) {
        return dlsym(handle, symbol);
    }
    inline int alpine_dlclose(alpine_dl_handle handle) {
        return dlclose(handle);
    }

#endif


// ============================================================================
//  Time
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    #include <ctime>

    #ifndef _TIMEVAL_DEFINED
    #define _TIMEVAL_DEFINED
    // MinGW may already define timeval via winsock2.h
    #endif

    // gettimeofday implementation for Windows
    inline int gettimeofday(struct timeval* tv, void* /*tz*/) {
        if (!tv) return -1;
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        // FILETIME is 100-nanosecond intervals since 1601-01-01
        // Unix epoch starts 1970-01-01: difference is 11644473600 seconds
        unsigned long long t = (static_cast<unsigned long long>(ft.dwHighDateTime) << 32)
                             | ft.dwLowDateTime;
        t -= 116444736000000000ULL;  // to Unix epoch
        tv->tv_sec  = static_cast<long>(t / 10000000ULL);
        tv->tv_usec = static_cast<long>((t / 10ULL) % 1000000ULL);
        return 0;
    }

    // localtime_r → localtime_s (argument order is swapped on Windows)
    inline struct tm* localtime_r(const time_t* timer, struct tm* result) {
        return (localtime_s(result, timer) == 0) ? result : nullptr;
    }

#else  // POSIX

    #include <sys/time.h>
    #include <ctime>

#endif


// ============================================================================
//  Process and signals
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    #include <process.h>

    using pid_t  = int;
    using uid_t  = unsigned int;
    using gid_t  = unsigned int;

    inline pid_t getpid() { return _getpid(); }

    // Minimal signal compatibility — only the constants used by Alpine
    #ifndef SIGHUP
    #define SIGHUP   1
    #endif
    #ifndef SIGINT
    #define SIGINT   2
    #endif
    #ifndef SIGQUIT
    #define SIGQUIT  3
    #endif
    #ifndef SIGPIPE
    #define SIGPIPE  13
    #endif
    #ifndef SIGTERM
    #define SIGTERM  15
    #endif
    #ifndef SIGUSR1
    #define SIGUSR1  10
    #endif
    #ifndef SIGUSR2
    #define SIGUSR2  12
    #endif

    // Stub sigset_t for Alpine's SignalSet class
    using sigset_t = unsigned long;
    inline int sigemptyset(sigset_t* s)         { *s = 0; return 0; }
    inline int sigfillset(sigset_t* s)          { *s = 0xFFFFFFFF; return 0; }
    inline int sigaddset(sigset_t* s, int sig)  { *s |= (1UL << sig); return 0; }
    inline int sigdelset(sigset_t* s, int sig)  { *s &= ~(1UL << sig); return 0; }

    // pthread_sigmask stub (no-op on Windows — threads don't mask signals)
    #define SIG_BLOCK   0
    #define SIG_UNBLOCK 1
    #define SIG_SETMASK 2
    inline int pthread_sigmask(int /*how*/, const sigset_t* /*set*/, sigset_t* /*old*/) {
        return 0;
    }

#else  // POSIX

    #include <signal.h>
    #include <unistd.h>

#endif


// ============================================================================
//  File system (stat)
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS
    #include <sys/stat.h>
    #include <io.h>
    #include <direct.h>

    // access() mode flags
    #ifndef R_OK
    #define R_OK 4
    #endif
    #ifndef W_OK
    #define W_OK 2
    #endif
    #ifndef X_OK
    #define X_OK 1
    #endif
    #ifndef F_OK
    #define F_OK 0
    #endif

    inline int alpine_access(const char* path, int mode) {
        return _access(path, mode & ~X_OK);  // Windows _access doesn't support X_OK
    }

#else  // POSIX

    #include <sys/stat.h>
    #include <sys/types.h>

    inline int alpine_access(const char* path, int mode) {
        return access(path, mode);
    }

#endif


// ============================================================================
//  Sleep
// ============================================================================

#ifdef ALPINE_PLATFORM_WINDOWS

    inline void alpine_usleep(unsigned int usec) {
        Sleep(usec / 1000);  // Windows Sleep is millisecond granularity
    }

#else  // POSIX

    inline void alpine_usleep(unsigned int usec) {
        usleep(usec);
    }

#endif
