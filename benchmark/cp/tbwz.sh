#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bin/tbwzip.x -c $2 $3
elif [ "$1" = "d" ]; then	#decompress infile
	bin/tbwzip.x -d $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	cd ..;make tbwzip.x
	cd benchmark;cp ../tbwzip.x bin/tbwzip.x
else
	exit 1
fi
