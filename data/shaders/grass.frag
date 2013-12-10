/*--- GENERIC HEADER -----------------------------------------------------------------------------*/

varying vec3 nor;
uniform float far;
uniform float objectid;

const float near = 1.0;

vec4 encdepth(float v) {
	vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
	enc = fract(enc);
	enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
	return enc;
}

/*--- END OF GENERIC HEADER ----------------------------------------------------------------------*/


uniform sampler2D tex;

void main()
{
	vec4 color = texture2D(tex, gl_TexCoord[0].st);




/*--- GENERIC FOOTER -----------------------------------------------------------------------------*/

	float linear_z = (2.0 * near) / (far + near - gl_FragCoord.z * (far - near));
	// Tune for better inside range without losing outdoors
	linear_z *= 2.0;


	gl_FragData[0] = color;	
	gl_FragData[1] = vec4(nor, linear_z);
	gl_FragData[2] = vec4(encdepth(gl_FragCoord.z).xyz, objectid);

/*--- END OF GENERIC FOOTER ----------------------------------------------------------------------*/
}
