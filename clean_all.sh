PARA=6

echo "------------------------------"
echo " Cleaning full source tree..."
echo "------------------------------"
echo ""


echo " - Cleaning for all targets and variations..."
make clean
make DEBUG=true clean
make PROFILE=true clean
make CONCURRENT=true clean
make CONCURRENT=true DEBUG=true clean
make CONCURRENT=true PROFILE=true clean


echo ""
echo ""
echo "-----------------"
echo " Clean complete."
echo "-----------------"
echo ""
