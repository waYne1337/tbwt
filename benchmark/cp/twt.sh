#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/twtzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/twtzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make twtzip.x
	cd benchmark;cp ../twtzip.x bin/twtzip.x
else
	exit 1
fi
