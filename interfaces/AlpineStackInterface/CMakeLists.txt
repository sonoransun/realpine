add_library(AlpineStackInterface
    AlpineStackInterface.cpp
)
add_library(Alpine::AlpineStackInterface ALIAS AlpineStackInterface)
target_include_directories(AlpineStackInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineStackInterface PUBLIC
    Alpine::AlpineProtocol
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)
