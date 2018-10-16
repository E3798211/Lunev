#!/bin/bash

for((i = 0; i < 50; i++)) 
do
	diff -q $i common.h
	rm $i
done
