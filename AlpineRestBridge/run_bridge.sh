#!/bin/bash

ALPINE_IP_ADDR=192.168.1.1
ALPINE_PORT=9000
REST_PORT=8080
BEACON_PORT=8089
DLNA_PORT=9090
MEDIA_DIR=/home/user/Videos

echo ""
echo "Starting ALPINE REST Bridge"
echo "----------------------------"
echo "IP Address ...: $ALPINE_IP_ADDR"
echo "Port .........: $ALPINE_PORT"
echo "REST Port ....: $REST_PORT"
echo "Beacon Port ..: $BEACON_PORT"
echo "DLNA Port ....: $DLNA_PORT"
echo "Media Dir ....: $MEDIA_DIR"
echo ""

export LD_LIBRARY_PATH=$ALPINE_ROOT/lib:$LD_LIBRARY_PATH

cat /dev/null > bridge.log

bin/AlpineRestBridge-r \
    --logFile bridge.log \
    --logLevel Debug \
    --ipAddress $ALPINE_IP_ADDR \
    --port $ALPINE_PORT \
    --restPort $REST_PORT \
    --beaconPort $BEACON_PORT \
    --dlnaPort $DLNA_PORT \
    --mediaDirectory $MEDIA_DIR \
    &

tail -f bridge.log
