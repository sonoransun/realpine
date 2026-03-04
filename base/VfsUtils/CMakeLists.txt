# Copyright (C) 2026 sonoransun — see LICENCE.txt
add_library(VfsUtils
    VfsNode.cpp
    QueryCache.cpp
    AccessTracker.cpp
    FuseOps.cpp
    AlpineFuse.cpp
    VfsStatsHandler.cpp
)
add_library(Alpine::VfsUtils ALIAS VfsUtils)
target_include_directories(VfsUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VfsUtils PUBLIC
    Alpine::AppUtils Alpine::ThreadUtils
    Alpine::AlpineStackInterface Alpine::DtcpStackInterface
    Alpine::AlpineProtocol
    FUSE::FUSE
)
target_compile_definitions(VfsUtils PUBLIC ALPINE_FUSE_ENABLED)
