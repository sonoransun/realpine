add_library(CorbaUtils
    CosNamingUtils.cpp
    OrbEventThread.cpp
    OrbUtils.cpp
)
add_library(Alpine::CorbaUtils ALIAS CorbaUtils)
target_include_directories(CorbaUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CorbaUtils PUBLIC
    Alpine::AppUtils
    Alpine::ThreadUtils
    TAO::TAO
    TAO::CosNaming
    TAO::SvcUtils
)
