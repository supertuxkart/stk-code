#!/bin/bash

# This script simplifies all challenges by removing any time 
# limit, position requirement, etc, and setting the number
# of laps to 0. This is meant to quickly test the story
# mode without having to fully play all challenges.

for i in data/challenges/*.challenge; do
    echo "Simplifying $i"
    cat $i | sed 's/position="[0-9]*"/position="99"/' \
           | sed 's/laps="[0-9]*"/laps="0"/'          \
           | sed 's/energy="[0-9]*"/energy="0"/'      \
           | sed 's/time="[0-9]*"/time="9999"/'       \
    > $i.new
    mv $i.new $i
done

for i in data/grandprix/*.grandprix; do
    echo "Simplyfing GP $i"
    cat $i | sed 's/laps="[0-9]*"/laps="0"/' > $i.new
    mv $i.new $i
done
echo
echo "All challenges simplified."
echo "PLEASE do not commit the changes back to our repository!"
echo "========================================================"

