#!/bin/bash

mkdir -p ../batch
count=1

#for numkarts in 4; do
#	for laps in 4 10; do
for ((mass = 50; mass <= 500; mass=$mass+10)) do
	sed -i -e "s/mass\ value\ =\ \"[0-9]*/mass\ value\ =\ \"$mass/g" ../stk-assets/karts/sara/kart.xml	
	for run in {1..10}; do
		echo $count/450
		./cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --numkarts=1  --with-profile --profile-laps=3 --kart=sara --ai=beastie,beastie,beastie --no-graphics > /dev/null
		grep "profile" ~/Library/Application\ Support/SuperTuxKart/stdout.log > ../batch/$mass.$run.txt
		let "count = $count + 1"
	done
done

for track in "cave" "city" "scotland" "jungle" "lighthouse" "hacienda" "fortmagma"; do
	./cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --track=$track --with-profile --profile-laps=1 > /dev/null
done