#!/usr/bin/env python

# This python script is used to remove paths from textures in the
# ac model files. Usage: strip_texture_path_from_models.py *.ac

import sys
import re
import os.path

reTexture = re.compile(r"^texture \"(.*)\"")
for sFile in sys.argv:
    f=open(sFile,"r")
    lLines=f.readlines()
    bNeedsWriting=0
    for i in range(len(lLines)):
        sLine = lLines[i]
        g=reTexture.match(sLine)
        if g:
            lLines[i]="texture \"%s\"\n"%os.path.basename(g.group(1))
            bNeedsWriting=1
    f.close()
    if bNeedsWriting:
        f=open(sFile,"w")
        f.writelines(lLines)
        f.close()
