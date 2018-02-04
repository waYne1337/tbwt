#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/wtzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/wtzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make wtzip.x
	cd benchmark;cp ../wtzip.x bin/wtzip.x
else
	exit 1
fi
