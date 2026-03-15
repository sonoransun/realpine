add_executable(rest_load_test rest_load_test.cpp)
target_link_libraries(rest_load_test PRIVATE
    Alpine::AppUtils
    Alpine::NetUtils
    Asio::Asio
    Alpine::Flags
)
