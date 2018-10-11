#!/bin/bash

for((i = 0; i < 5000; i++)) 
do
	diff -q $i /bin/bash
	rm $i
done
