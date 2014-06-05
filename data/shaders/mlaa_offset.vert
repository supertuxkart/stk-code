layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

in vec2 Position;
in vec2 Texcoord;

out vec4 offset[2];
out vec2 uv;

void main() {
	gl_Position = vec4(Position, 0., 1.);
	vec4 invy = vec4(Texcoord, Texcoord);
//	invy.y = 1.0 - invy.y;
	uv = invy.st;

	offset[0] = invy.xyxy + screen.xyxy * vec4(-1.0, 0.0, 0.0,  1.0);
	offset[1] = invy.xyxy + screen.xyxy * vec4( 1.0, 0.0, 0.0, -1.0);
}
