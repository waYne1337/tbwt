#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	bzip2 -9 -c -f -k $2 > $3
elif [ "$1" = "d" ]; then	#decompress infile
	bunzip2 -c -f -k $2 > $3
elif [ "$1" = "i" ]; then	#install compressor
	apt-get install bzip2
else
	exit 1
fi
