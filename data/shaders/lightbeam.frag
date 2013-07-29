// Jean-manuel clemencon supertuxkart
// Creates a cone lightbeam effect by smoothing edges
// Original idea: http://udn.epicgames.com/Three/VolumetricLightbeamTutorial.html
// TODO: Soft edges when it intesects geometry
// Some artefacts are still visible

uniform sampler2D main_texture;
uniform float transparency;
varying vec2 uv;
varying vec3 eyeVec;
varying vec3 normal;

void main()
{
	float inter = dot(normal, eyeVec);
	float m = texture2D(main_texture, vec2(0.5, uv.y)).r;
	float alpha = inter * inter * inter * inter * m;

	gl_FragColor = vec4(1.0, 1.0, 0.8, alpha);
}
