#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	gzip -c -f -k $2 > $3
elif [ "$1" = "d" ]; then	#decompress infile
	gunzip -c -f -k $2 > $3
elif [ "$1" = "i" ]; then	#install compressor
	apt-get install gzip
else
	exit 1
fi
