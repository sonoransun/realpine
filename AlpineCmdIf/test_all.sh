# Base options
BINARY=bin/AlpineCmdIf-r
LOG_FILE=cmdif.log
LOG_LEVEL=Debug
NAMING_IOR=file:///projects/alpine/release/runtime/corba/naming-service/ior.txt
INTERFACE_CONTEXT="/alpine/servers/logic"

# Command options
IP_ADDR=192.168.1.1
PORT=60000
PEER_ID=1
SUBNET_IP=192.168.1.0
SUBNET_MASK=255.255.255.0
LIMIT=100
GROUP_NAME=TestGroup
DESCRIPTION="detailed decription of some kind"
ALIAS="some peer"
QUERY_STRING="the search for decentralized networks"
OPTIONS=0
QUERY_ID=1
SLEEP=5

# Error checking
DO_CHECK=0
CMD_ERROR=3



echo ""
cat /dev/null > cmdif.log
echo "Full Test Script" >> cmdif.log
echo "----------------" >> cmdif.log


# Add peer
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command addDtcpPeer \
    --ipAddress $IP_ADDR \
    --port $PORT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Get peer ID
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getDtcpPeerId \
    --ipAddress $IP_ADDR \
    --port $PORT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Get peer status 
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getDtcpPeerStatus \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Activate peer
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command activateDtcpPeer \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Deactivate peer
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command deactivateDtcpPeer \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Ping peer
sleep $SLEEP
$BINARY \
    --verbose  --logFile $LOG_FILE  --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command pingDtcpPeer \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Exclude host 
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command excludeHost \
    --ipAddress $IP_ADDR \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Exclude subnet
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command excludeSubnet \
    --subnetIpAddress $SUBNET_IP \
    --subnetMask $SUBNET_MASK \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Allow host
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command allowHost \
    --ipAddress $IP_ADDR \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# Allow subnet
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command allowSubnet \
    --subnetIpAddress $SUBNET_IP \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# listExcludedHosts
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command listExcludedHosts \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# listExcludedSubnets
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command listExcludedSubnets \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# natDiscovery
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command natDiscovery \
    --required true \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# natDiscoveryQuery
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command natDiscoveryQuery \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setDataSendingLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setDataSendingLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getDataSendingLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getDataSendingLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setStackThreadLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setStackThreadLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getStackThreadLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getStackThreadLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setReceiveBufferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setReceiveBufferLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getReceiveBufferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getReceiveBufferLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setSendBufferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setSendBufferLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getSendBufferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getSendBufferLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getBufferStats 
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getBufferStats \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setTotalTransferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setTotalTransferLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getTotalTransferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getTotalTransferLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# setPeerTransferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command setPeerTransferLimit \
    --limit $LIMIT \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getPeerTransferLimit
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getPeerTransferLimit \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getTransferStats
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getTransferStats \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getUserGroupList
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getUserGroupList \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# createUserGroup
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command createUserGroup \
    --groupName $GROUP_NAME \
    --description $DESCRIPTION \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# destroyUserGroup 
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command destroyUserGroup \
    --groupName $GROUP_NAME \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getPeerUserGroupList
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getPeerUserGroupList \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# addPeerToGroup
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command addPeerToGroup \
    --peerId $PEER_ID \
    --groupName $GROUP_NAME \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# removePeerFromGroup
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command removePeerFromGroup \
    --peerId $PEER_ID \
    --groupName $GROUP_NAME \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getExtendedPeerList
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getExtendedPeerList \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getPeerInformation
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getPeerInformation \
    --peerId $PEER_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;


    

# updatePeerInformation
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command updatePeerInformation \
    --peerId $PEER_ID \
    --alias $ALIAS \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# startQuery
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command startQuery \
    --queryString $QUERY_STRING \
    --options $OPTIONS \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getQueryStatus
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getQueryStatus \
    --queryId $QUERY_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# pauseQuery 
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command pauseQuery \
    --queryId $QUERY_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# resumeQuery
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command resumeQuery \
    --queryId $QUERY_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# cancelQuery
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command cancelQuery \
    --queryId $QUERY_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;




# getQueryResults
sleep $SLEEP
$BINARY \
    --verbose --logFile $LOG_FILE --logLevel $LOG_LEVEL \
    --interfaceContext $INTERFACE_CONTEXT -ORBInitRef NameService=$NAMING_IOR \
    --command getQueryResults \
    --queryId $QUERY_ID \

RET_VAL=$?
# Always check for initialization / operation errors
if [ $RET_VAL != 0 ]; then
    if [ $RET_VAL != $CMD_ERROR ]; then
        echo "Command initialization/argument error!  Ending tests...";exit 1; 
    fi;
    if [ $DO_CHECK == 1 ]; then
        echo "Command FAILED! Ending tests...";exit 1;
    else
        echo "Command failed, continuing tests...";
    fi;
fi;







