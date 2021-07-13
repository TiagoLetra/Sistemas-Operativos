# Sistemas Operativos, DEI/IST/ULisboa 2020-21
# ShellScript

#!/bin/bash

trap "exit" INT
if [ ! $# = 3 ] ; then
	echo "Not enough arguments"
	exit
fi

inputDir=$1
outDir=$2
maxThreads=$3

[[ ! -d $outDir ]] && mkdir $outDir

for f in $1/* ; do

	for t in $(seq 1 $maxThreads) ; do
	echo "InputFile=$f NumThreads=$3"
		./tecnicofs $f "$outDir/${f##*/}-${t}.txt" $t | grep "TecnicoFS"
	done
done
