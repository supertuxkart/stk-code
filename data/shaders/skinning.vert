// skinning.vert

#define MAX_JOINT_NUM 36
#define MAX_LIGHT_NUM 8

uniform mat4 JointTransform[MAX_JOINT_NUM];

void main() 
{
	int index;
	vec4 ecPos;
	vec3 normal;
	vec3 light_dir;
	float n_dot_l;
	float dist;

	mat4 ModelTransform = gl_ModelViewProjectionMatrix;
	 
	index = int(gl_Color.r * 255.99);
	mat4 vertTran = JointTransform[index - 1];
	
	index = int(gl_Color.g * 255.99);
	if(index > 0)
		vertTran += JointTransform[index - 1];

	index = int(gl_Color.b * 255.99);
	if(index > 0)
		vertTran += JointTransform[index - 1];
		
	index = int(gl_Color.a * 255.99);
	if(index > 0)
		vertTran += JointTransform[index - 1];
		
	ecPos = gl_ModelViewMatrix * vertTran * gl_Vertex;
	
	normal = normalize(gl_NormalMatrix * mat3(vertTran) * gl_Normal);
	
	gl_FrontColor = vec4(0,0,0,0);
	for(int i = 0;i < MAX_LIGHT_NUM;i++)
	{
		light_dir = vec3(gl_LightSource[i].position-ecPos);
		n_dot_l = max(dot(normal, normalize(light_dir)), 0.0);
		dist = length(light_dir);
		n_dot_l *= 1.0 / (gl_LightSource[0].constantAttenuation + gl_LightSource[0].linearAttenuation * dist);
		gl_FrontColor += gl_LightSource[i].diffuse * n_dot_l;
	}
	gl_FrontColor = clamp(gl_FrontColor,0.3,1.0);
	

	ModelTransform *= vertTran;
		
	gl_Position = ModelTransform * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	
	/* 
	// Reflections.
	vec3 r = reflect( ecPos.xyz , normal );
	float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0) );
	gl_TexCoord[1].s = r.x/m + 0.5;
	gl_TexCoord[1].t = r.y/m + 0.5;
	*/
}
