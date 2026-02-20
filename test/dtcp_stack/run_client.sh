echo ""
echo "Starting Client"
cat /dev/null > client.log
bin/dtcpStackTestClient-r client.log 4 192.168.1.1 4000 127.0.0.1 3000 1
echo "Finished."
echo ""
