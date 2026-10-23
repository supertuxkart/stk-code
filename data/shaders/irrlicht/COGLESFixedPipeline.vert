#version 310 es

/* Ins */

in vec3 inVertexPosition;
in vec3 inVertexNormal;
in vec4 inVertexColor;
in vec2 inTexCoord0;

/* Uniforms */

uniform mat4 uMvpMatrix;

uniform vec2 uTextureTrans0;

/* Outs */

out vec2 varTexCoord0;
out vec4 varVertexColor;
out float varEyeDist;

void main(void)
{
	gl_Position = uMvpMatrix * vec4(inVertexPosition,1.0);

	varTexCoord0 = inTexCoord0 + uTextureTrans0;
	varVertexColor = inVertexColor.zyxw;
	varEyeDist = length((uMvpMatrix * vec4(inVertexPosition, 1.0)).xyz);
}
