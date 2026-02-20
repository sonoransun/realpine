PARA=3

echo "------------------------------"
echo " Building source ..."
echo "------------------------------"
echo ""


echo " - Concurrent build"
nice -n 40 make -j $PARA CONCURRENT=true

echo ""
echo ""
echo "-----------------"
echo " Build complete."
echo "-----------------"
echo ""
