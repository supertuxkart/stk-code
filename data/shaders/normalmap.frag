// By http://content.gpwiki.org/index.php/OpenGL:Tutorials:GLSL_Bump_Mapping
// Released under GNU FDL license, without invariant (so DFSG-compliant, see
// http://wiki.debian.org/DFSGLicenses#Exception)

uniform sampler2D BumpTex; //The bump-map 
 uniform sampler2D DecalTex; //The texture
 varying vec4 passcolor; //Receiving the vertex color from the vertex shader
 varying vec3 LightDir; //Receiving the transformed light direction 
 void main() 
 {
   vec4 LightDirTransformed4 = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(LightDir[0], LightDir[1], LightDir[2], 0);
   vec3 LightDirTransformed = vec3(LightDirTransformed4[0], LightDirTransformed4[1], LightDirTransformed4[2]);
   
   //Get the color of the bump-map
   vec3 BumpNorm = vec3(texture2D(BumpTex, gl_TexCoord[0].xy));
   //Get the color of the texture
   vec3 DecalCol = vec3(texture2D(DecalTex, gl_TexCoord[0].xy));
   //Expand the bump-map into a normalized signed vector
   BumpNorm = (BumpNorm -0.5) * 2.0;
   //Find the dot product between the light direction and the normal
   float NdotL = max(dot(BumpNorm, LightDirTransformed), 0.0) / 3.0 * 2.1 + 0.5;
   //Calculate the final color gl_FragColor
   vec3 diffuse = NdotL * passcolor.xyz * DecalCol;
   //Set the color of the fragment...  If you want specular lighting or other types add it here
   gl_FragColor = vec4(diffuse, passcolor.w);
 }