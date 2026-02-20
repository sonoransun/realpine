ALPINE_IP_ADDR=192.168.1.1
ALPINE_PORT=9000

ORB_IP_ADDR=192.168.1.1
ORB_PORT=7060
ORB_ENDPOINT=iiop://$ORB_IP_ADDR:$ORB_PORT

NAMING_IP_ADDR=192.168.1.1
NAMING_PORT=7050

NAMING_REF=file:///projects/alpine/release/runtime/corba/naming-service/ior.txt
#NAMING_REF=iiop://$NAMING_IP_ADDR:$NAMING_PORT



echo ""
echo "Starting ALPINE Server"
echo "----------------------"
echo "IP Address .: $IP_ADDR"
echo "Port .......: $PORT"
echo "ORB ........: $ORB_ENDPOINT"
echo "NS Ref .....: $NAMING_REF"
echo ""

cat /dev/null > server.log

#    -ORBEndPoint $ORB_ENDPOINT \

bin/AlpineServer-r \
    --logFile server.log \
    --logLevel Debug \
    --ipAddress $ALPINE_IP_ADDR \
    --port $ALPINE_PORT \
    --interfaceContext "/alpine/servers/logic" \
    -ORBInitRef NameService=$NAMING_REF \
    &

tail -f server.log
