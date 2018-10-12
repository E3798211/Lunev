#!/bin/bash

for((i = 0; i < 5000; i++)) 
do
	diff -q $i common.h
	rm $i
done
