#!/bin/bash

for((i = 0; i < 5000; i++))
do
	./writer common.h 	&
	./reader	>$i	    &
done