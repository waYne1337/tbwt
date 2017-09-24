#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/bwzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/bwzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make bwzip.x
	cd benchmark;cp ../bwzip.x bin/bwzip.x
else
	exit 1
fi
