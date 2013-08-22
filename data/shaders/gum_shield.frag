// Jean-manuel clemencon (c) supertuxkart 2013
// bubble gum shield
// TODO: Add a nice texture and soft edges when intersect with geometry

uniform sampler2D tex;
uniform float transparency;

varying vec2 uv;
varying vec3 eyeVec;
varying vec3 normal;

void main()
{
	float inter = dot(normal, eyeVec);
	float m = texture2D(tex, vec2(0.5, uv.y)).r;
	inter = 1.0 - inter;
	float alpha = inter + 1.0;// * m;

	gl_FragColor = vec4(0.8, 0.16, 0.48, alpha);
}
