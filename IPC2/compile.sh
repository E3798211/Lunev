#!/bin/bash

gcc -Wall common.c reader.c -o reader
gcc -Wall common.c writer.c -o writer