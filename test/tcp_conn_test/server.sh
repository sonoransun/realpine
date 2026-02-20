cat /dev/null > server.log

./bin/tcpServer-r \
   --logLevel Info \
   --logFile server.log \
   --ipAddress 192.168.1.1 \
   --port 9090
