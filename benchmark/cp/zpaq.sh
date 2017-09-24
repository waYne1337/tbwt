#!/bin/bash
#check args
if [ "$1" = "c" ]; then		#compress infile
	zpaq qisc $3 $2
elif [ "$1" = "d" ]; then	#decompress infile
	zpaq qx $2 $3
elif [ "$1" = "i" ]; then	#install compressor
	apt-get install zpaq
else
	exit 1
fi
