add_executable(rest_load_test rest_load_test.cpp)
target_link_libraries(rest_load_test PRIVATE
    Alpine::AppUtils
    Alpine::NetUtils
    Asio::Asio
    Alpine::Flags
)

# NOT registered with ctest.  rest_load_test points HTTP requests at a live
# REST server (default 127.0.0.1:8080).  The natural target, AlpineRestBridge,
# is a heavyweight binary that boots the full Alpine peer stack — DTCP, mDNS,
# SSDP, optional Tor/UPnP/FUSE, etc. — which is wholly unsuitable for an
# unattended ctest run on shared CI hosts (multicast, port collisions, slow
# startup).  Run it manually after starting your own bridge:
#   ./bin/AlpineRestBridge --logFile rest.log --logLevel Info \
#                          --ipAddress 127.0.0.1 --port 0 &
#   ./bin/rest_load_test --host 127.0.0.1 --port 8080 \
#                        --connections 4 --requests-per-conn 50 --path /status
