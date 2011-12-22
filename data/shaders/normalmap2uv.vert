// By http://content.gpwiki.org/index.php/OpenGL:Tutorials:GLSL_Bump_Mapping
// Released under GNU FDL license, without invariant (so DFSG-compliant, see
// http://wiki.debian.org/DFSGLicenses#Exception)

varying vec4 passcolor; //The vertex color passed
 varying vec3 LightDir; //The transformed light direction, to pass to the fragment shader
 attribute vec3 tangent; //The inverse tangent to the geometry
 attribute vec3 binormal; //The inverse binormal to the geometry
 uniform vec3 lightdir; //The direction the light is shining
 void main() 
 {
   //Put the color in a varying variable
   passcolor = gl_Color;
   //Put the vertex in the position passed
   gl_Position = ftransform(); 
   //Construct a 3x3 matrix from the geometry’s inverse tangent, binormal, and normal
   mat3 rotmat = mat3(tangent,binormal,gl_Normal);
   //Rotate the light into tangent space
   LightDir = rotmat * normalize(lightdir);
   //Normalize the light
   normalize(LightDir); 
   //Use the first set of texture coordinates in the fragment shader 
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_TexCoord[1] = gl_MultiTexCoord1;
 }
