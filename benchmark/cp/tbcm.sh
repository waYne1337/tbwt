#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/tbcmzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/tbcmzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make tbcmzip.x
	cd benchmark;cp ../tbcmzip.x bin/tbcmzip.x
else
	exit 1
fi
