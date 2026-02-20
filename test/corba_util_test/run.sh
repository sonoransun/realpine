IOR_FILE=file:///projects/alpine/release/runtime/corba/naming-service/ior.txt

echo ""
echo "Starting Application"
cat /dev/null > test.log

bin/CorbaUtilsTest-r \
    --logFile test.log \
    --logLevel Debug \
    --context "/test0/alpine/corba/aContext" \
    --binding "this/shoud/be/relative/aObject" \
    -ORBInitRef NameService=$IOR_FILE


echo "Finished."
echo ""
