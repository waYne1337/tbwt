#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/bcmzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/bcmzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make bcmzip.x
	cd benchmark;cp ../bcmzip.x bin/bcmzip.x
else
	exit 1
fi
