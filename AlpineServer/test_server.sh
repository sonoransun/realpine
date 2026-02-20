LOG_FILE="test-${1}.log"
IP_ADDR=192.168.1.1
PORT=$2
CONTEXT="/alpine/servers/test-${1}"

echo ""
echo "Starting -TEST- ALPINE Server"
echo "-----------------------------"
echo "Log File ...: $LOG_FILE"
echo "IP Address .: $IP_ADDR"
echo "Port .......: $PORT"
echo "NS Context .: $CONTEXT"
echo ""

cat /dev/null > $LOG_FILE

bin/AlpineServer-r \
    --logFile $LOG_FILE \
    --logLevel Debug \
    --ipAddress $IP_ADDR \
    --port $PORT \
    --interfaceContext $CONTEXT \
    -ORBInitRef NameService=file:///projects/runtime/naming_service/ior.txt \
    &

tail -f $LOG_FILE
