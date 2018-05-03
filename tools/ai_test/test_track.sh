#!/bin/bash

for track in abyss candela_city cocoa_temple cornfield_crossing fortmagma gran_paradiso_island greenvalley hacienda lighthouse mansion mines minigolf olivermath sandtrack scotland snowmountain snowtuxpeak stk_enterprise volcano_island xr591 zengarden; do
 echo "Testing $track"
   $1 --log=0 -R     \
       --aiNP=nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok,nolok \
       --track=$track --difficulty=3 --type=1 --test-ai=2   \
       --profile-laps=10  --no-graphics > stdout.$track
done
