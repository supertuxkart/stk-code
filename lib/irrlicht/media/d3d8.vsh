; part of the Irrlicht Engine Shader example.
; This Direct3D9 vertex shader will be loaded by the engine.
; Please note that these example shaders don't do anything really useful. 
; They only demonstrate that shaders can be used in Irrlicht.

vs.1.1

; transpose and transform position to clip space 
mul r0, v0.x, c4      
mad r0, v0.y, c5, r0   
mad r0, v0.z, c6, r0   
add oPos, c7, r0       

; transform normal 
dp3 r1.x, v1, c0  
dp3 r1.y, v1, c1  
dp3 r1.z, v1, c2  

; renormalize normal 
dp3 r1.w, r1, r1  
rsq r1.w, r1.w    
mul r1, r1, r1.w  

; calculate light vector 
m4x4 r6, v0, c10      ; vertex into world position
add r2, c8, -r6       ; vtxpos - lightpos

; normalize light vector 
dp3 r2.w, r2, r2  
rsq r2.w, r2.w    
mul r2, r2, r2.w  

; calculate light color 
dp3 r3, r1, r2       ; dp3 with negative light vector 
lit r5, r3           ; clamp to zero if r3 < 0, r5 has diffuce component in r5.y
mul oD0, r5.y, c9    ; ouput diffuse color 
mov oT0, v3          ; store texture coordinates
