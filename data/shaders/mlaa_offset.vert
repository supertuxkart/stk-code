in vec2 Position;
in vec2 Texcoord;

out vec4 offset[2];
out vec2 uv;

void main() {
	gl_Position = vec4(Position, 0., 1.);
	vec4 invy = vec4(Texcoord, Texcoord);
//	invy.y = 1.0 - invy.y;
	uv = invy.st;

	offset[0] = invy.xyxy + vec4(-1.0, 0.0, 0.0, 1.0) / screen.xyxy;
	offset[1] = invy.xyxy + vec4( 1.0, 0.0, 0.0, -1.0) / screen.xyxy;
}
