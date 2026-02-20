add_library(CorbaAdmin
    CorbaAdmin.cpp
)
add_library(Alpine::CorbaAdmin ALIAS CorbaAdmin)
target_include_directories(CorbaAdmin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CorbaAdmin PUBLIC
    Alpine::AlpineCorbaServer
    Alpine::CorbaUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
)
