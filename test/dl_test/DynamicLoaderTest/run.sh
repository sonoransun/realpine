cat /dev/null > loader.log

./bin/dynamicLoaderTest-r \
   --logLevel Debug \
   --logFile loader.log \
   --libraryName $ALPINE_ROOT/lib/libDynTestLib-r.so \
   --entryFunction createDynamicObject \
   --numObjects 10
