

uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
uniform sampler2D tex_detail4;
varying vec3 normal;
varying vec3 lightdir2;
varying vec4 vertex_color;

void main()
{
	vec4 splatting = texture2D(tex_layout, gl_TexCoord[0].st);
    vec4 detail0 = texture2D(tex_detail0, gl_TexCoord[1].st);
    vec4 detail1 = texture2D(tex_detail1, gl_TexCoord[1].st);
    vec4 detail2 = texture2D(tex_detail2, gl_TexCoord[1].st);
    vec4 detail3 = texture2D(tex_detail3, gl_TexCoord[1].st);
    vec4 detail4 = texture2D(tex_detail4, gl_TexCoord[1].st);
    
    gl_FragColor = (splatting.r * detail0 +
                    splatting.g * detail1 +
                    splatting.b * detail2 +
                    (1.0 - splatting.r - splatting.g - splatting.b) * detail3 +
                    (1.0 - splatting.a) * detail4)
                    * min(1.0, 0.2 + dot(lightdir2, normal)) * vertex_color; // 0.2 is the ambient light.
}
