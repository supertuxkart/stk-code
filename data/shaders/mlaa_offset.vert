#version 330 compatibility
uniform vec2 PIXEL_SIZE;
uniform mat4 ModelViewProjectionMatrix;

out vec4 offset[2];
out vec2 uv;

void main() {
	gl_Position = ModelViewProjectionMatrix * gl_Vertex;
	vec4 invy = gl_MultiTexCoord0;
//	invy.y = 1.0 - invy.y;
	uv = invy.st;

	offset[0] = invy.xyxy + PIXEL_SIZE.xyxy * vec4(-1.0, 0.0, 0.0,  1.0);
	offset[1] = invy.xyxy + PIXEL_SIZE.xyxy * vec4( 1.0, 0.0, 0.0, -1.0);
}
