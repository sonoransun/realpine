cat /dev/null > test.log
./bin/memTest-r test.log 4 $1 &
tail -f test.log
