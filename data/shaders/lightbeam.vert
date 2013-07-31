// Jean-manuel clemencon supertuxkart
// Creates a cone lightbeam effect by smoothing edges

varying vec2 uv;
varying vec3 eyeVec;
varying vec3 normal;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 viewp = gl_ModelViewMatrix * gl_Vertex;

	eyeVec = normalize(-viewp).xyz;
	normal = gl_NormalMatrix * gl_Normal;

	gl_Position = ftransform();

	uv = gl_TexCoord[0].st;
}
