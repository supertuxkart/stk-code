#version 330 compatibility
uniform mat4 ModelViewProjectionMatrix;

void main() {
	gl_Position = (ModelViewProjectionMatrix * gl_Vertex).xyww;
}
