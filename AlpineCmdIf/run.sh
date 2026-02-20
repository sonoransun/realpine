IP_ADDR=192.168.1.1
PORT=$1
NAMING_IOR=file:///projects/alpine/release/runtime/corba/naming-service/ior.txt

echo ""
cat /dev/null > cmdif.log

bin/AlpineCmdIf-r \
    --logFile cmdif.log \
    --logLevel Debug \
    --interfaceContext "/alpine/servers/logic" \
    --command addDtcpPeer \
    --ipAddress $IP_ADDR \
    --port $1 \
    --verbose \
    -ORBInitRef NameService=$NAMING_IOR \


echo ""
