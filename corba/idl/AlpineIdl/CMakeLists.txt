tao_idl_compile(${CMAKE_CURRENT_SOURCE_DIR}/Alpine.idl ${CMAKE_CURRENT_BINARY_DIR})

add_library(AlpineIdl
    ${CMAKE_CURRENT_BINARY_DIR}/AlpineC.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AlpineS.cpp
)
add_library(Alpine::AlpineIdl ALIAS AlpineIdl)
target_include_directories(AlpineIdl PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(AlpineIdl PUBLIC TAO::TAO TAO::CosNaming TAO::SvcUtils Alpine::Flags)
