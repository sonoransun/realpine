cat /dev/null > test.log
./bin/memTestLean-r test.log 4 $1 &
tail -f test.log
