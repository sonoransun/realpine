PARA=3

echo "------------------------------"
echo " Building full source tree..."
echo "------------------------------"
echo ""


echo " - Concurrent build"
make -j $PARA CONCURRENT=true

echo " - Concurrent Debug build"
make -j $PARA CONCURRENT=true DEBUG=true

echo " - Concurrent Profile build"
make -j $PARA CONCURRENT=true PROFILE=true

echo " - Normal build"
make -j $PARA

echo " - Debug build"
make -j $PARA DEBUG=true

echo " - Profile build"
make -j $PARA PROFILE=true


echo ""
echo ""
echo "-----------------"
echo " Build complete."
echo "-----------------"
echo ""
