#!/bin/bash

for file in $1/*
do
  prefix=*/ #remove directory
	suffix=.*
	temp=${file##$prefix}
  filename=${temp%$suffix}
	echo InputFile=$file NumThreads=1
	echo "$(./tecnicofs-nosync $file $2/$filename-1.txt 1)" | grep "TecnicoFS completed in "
	for threads in $(seq 2 $3)
	do
		echo InputFile=$file NumThreads=$threads
		echo "$(./tecnicofs-mutex $file $2/$filename-$threads.txt $threads)" | grep "TecnicoFS completed in "
	done
done