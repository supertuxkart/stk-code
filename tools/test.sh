#!/bin/bash

mkdir -p ../../batch
#tracks='snowmountain city lighthouse olivermath hacienda startrack farm zengarden'
#karts='gnu tux sara elephpant'
laps=4

#for track in $tracks; do
	#for kart in $karts; do		
		for run in {901..1500}; do
			for lap in $laps; do
				./../cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --numkarts=4  --track=snowmountain --with-profile --profile-laps=4 --kart=gnu --ai=sara,tux,elephpant --no-graphics > /dev/null
				#./cmake_build/bin/supertuxkart.app/Contents/MacOS/supertuxkart -R  --mode=3 --numkarts=4  --track=$track --with-profile --profile-laps=$lap --kart=$kart --ai=beastie,beastie,beastie --no-graphics > /dev/null
				#grep "profile" ~/Library/Application\ Support/SuperTuxKart/stdout.log > ../batch/$kart.$track.$run.txt
				grep "profile" ~/Library/Application\ Support/SuperTuxKart/stdout.log > ../../batch/faceoff.$run.txt
			done
		done
#	done
#done