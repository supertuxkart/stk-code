#!/bin/bash

for track in abyss cocoa_temple fortmagma greenvalley lighthouse olivermath snowmountain stk_enterprise zengarden hacienda mansion snowtuxpeak farm mines sandtrack city gran_paradiso_island minigolf scotland xr591; do
 echo "Testing $track"
   $1 --log=0 -R     \
       --ai=nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok \
       --track=$track --difficulty=2 --type=1 --test-ai=2   \
       --profile-laps=10  --no-graphics > stdout.$track
done
