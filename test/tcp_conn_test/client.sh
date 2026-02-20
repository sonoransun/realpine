cat /dev/null > client.log

./bin/tcpClient-r \
   --logLevel Info \
   --logFile client.log \
   --ipAddress 192.168.1.1 \
   --port 9090 \
   --transports 1 \
   --method Async
