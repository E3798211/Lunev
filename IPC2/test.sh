#!/bin/bash

for((i = 0; i < 50; i++))
do
	./writer common.h 	&
	./reader	>$i	    &
done