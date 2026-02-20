rm test.log *.out
./mapTest-prof test.log 4 $1 40
gprof -b -p mapTest-prof | sed 's/<[^:]*>/<>/g'
