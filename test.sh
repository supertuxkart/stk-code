#!/bin/bash

#TODO test multiple tracks
#TODO fix escape characters

mkdir -p ../batch

for run in {1..50}; do
    for num in {1..4}; do
		for laps in {2..7}; do
			./cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --numkarts=$num  --with-profile --profile-laps=$laps  --no-graphics | grep "profile" > ../batch/$run.$num.$laps.txt 
		done
	done
done