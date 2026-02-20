echo ""
echo "Starting Server"
cat /dev/null > server.log
bin/dtcpStackTestServer-r --logFile server.log --logLevel Debug --ipAddress 192.168.1.1 --port 4000
echo "Finished."
echo ""
