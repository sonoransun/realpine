# AppUtils and ThreadUtils have a mutual dependency (Log.h includes Mutex.h).
# Break it with an INTERFACE header target so AppUtils gets include paths
# without creating a circular link dependency.
add_library(ThreadUtilsHeaders INTERFACE)
target_include_directories(ThreadUtilsHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ThreadUtils
    AutoThread.cpp
    Mutex.cpp
    MutexLock.cpp
    ReadLock.cpp
    ReadWriteSem.cpp
    SysThread.cpp
    ThreadSigMask.cpp
    ThreadUtils.cpp
    WriteLock.cpp
)
add_library(Alpine::ThreadUtils ALIAS ThreadUtils)
target_include_directories(ThreadUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ThreadUtils PUBLIC Alpine::AppUtils Alpine::SysUtils)
