!!ARBvp1.0
# part of the Irrlicht Engine Shader example.
# Please note that these example shaders don't do anything really useful. 
# They only demonstrate that shaders can be used in Irrlicht.

#input
ATTRIB InPos = vertex.position;
ATTRIB InColor = vertex.color;
ATTRIB InNormal = vertex.normal;
ATTRIB InTexCoord = vertex.texcoord;

#output
OUTPUT OutPos = result.position;
OUTPUT OutColor = result.color;
OUTPUT OutTexCoord = result.texcoord;

PARAM MVP[4] = { state.matrix.mvp }; # modelViewProjection matrix.
TEMP Temp;
TEMP TempColor;
TEMP TempNormal;
TEMP TempPos;

#transform position to clip space 
DP4 Temp.x, MVP[0], InPos;
DP4 Temp.y, MVP[1], InPos;
DP4 Temp.z, MVP[2], InPos;
DP4 Temp.w, MVP[3], InPos;

#transform normal
DP3 TempNormal.x, InNormal.x, program.local[0];
DP3 TempNormal.y, InNormal.y, program.local[1]; 
DP3 TempNormal.z, InNormal.z, program.local[2];

#renormalize normal
DP3 TempNormal.w, TempNormal, TempNormal;  
RSQ TempNormal.w, TempNormal.w;    
MUL TempNormal, TempNormal, TempNormal.w;

# calculate light vector 
DP4 TempPos.x, InPos, program.local[10];   # vertex into world position
DP4 TempPos.y, InPos, program.local[11];
DP4 TempPos.z, InPos, program.local[12];
DP4 TempPos.w, InPos, program.local[13];

ADD TempPos, program.local[8], -TempPos;    # vtxpos - lightpos

# normalize light vector
DP3 TempPos.w, TempPos, TempPos;  
RSQ TempPos.w, TempPos.w;    
MUL TempPos, TempPos, TempPos.w;

# calculate light color
DP3 TempColor, TempNormal, TempPos;    # dp3 with negative light vector 
LIT OutColor, TempColor;  # clamp to zero if r3 < 0, r5 has diffuce component in r5.y
MUL OutColor, TempColor.y, program.local[9]; # ouput diffuse color 
MOV OutColor.w, 1.0;          # we want alpha to be always 1
MOV OutTexCoord, InTexCoord; # store texture coordinate
MOV OutPos, Temp;

END