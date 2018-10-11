#!/bin/bash

for((i = 0; i < 5000; i++))
do
	./writer common.c 	&
	./reader	>$i	    &
done

#for((i = 0; i < 50; i++))
#do
#	echo $i
#	sleep 5
#done

#echo $(pgrep writer | xargs n_processes)

#for((i = 0; i < 200; i++)) 
#do
#	diff -q $i /bin/bash
#	rm $i
#done

