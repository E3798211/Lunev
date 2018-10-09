#!/bin/bash

for((i = 0; i < 100; i++))
do
	./writer /bin/bash 	&
	./reader	>$i	    &
done

for((i = 0; i < 30; i++))
do
	echo $i
	sleep 5
done

for((i = 0; i < 100; i++)) 
do
	diff -q $i /bin/bash
	rm $i
done

