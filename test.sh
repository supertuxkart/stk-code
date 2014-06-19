#!/bin/bash

mkdir -p ../batch

tracks='snowmountain city lighthouse olivermath hacienda startrack farm zengarden'
karts='beastie sara elephpant tuxley'
laps=1

for track in $tracks; do
	#beastie is the current kart
	#sara is the light kart
	#tuxley is the medium kart
	#elephpant is the heavy kart
	for kart in $karts; do		
		for run in {1..50}; do
			for lap in $laps; do
				./cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --numkarts=1  --track=$track --with-profile --profile-laps=$lap --kart=$kart --ai=beastie,beastie,beastie --no-graphics > /dev/null
				grep "profile" ~/Library/Application\ Support/SuperTuxKart/stdout.log > ../batch/$track.$kart.$run.txt
			done
		done
	done
done
			
		




#for track in ../stk-assets/tracks/*/track.xml; do	
#	grep "internal" "$track"
#	if grep -q "internal" "$track" ; then
#		continue;
#	else
#		if grep -q "arena" "$track" ; then
#			continue;
#		else
#			if grep -q "soccer" "$track" ; then
#				continue;
#			else
#				a=${track%%/track.xml};
#				trackname=${a##../stk-assets/tracks/};
#			fi
#		fi
#	fi
#	if [ $trackname = "minel" ] || [ $trackname = "mines" ]; then
#		continue;
#	fi