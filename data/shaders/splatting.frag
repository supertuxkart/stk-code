

uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
uniform sampler2D tex_detail4;
varying vec3 normal;
varying vec3 lightdir2;

void main()
{
	vec4 layout = texture2D(tex_layout, gl_TexCoord[0].st);
    vec4 detail0 = texture2D(tex_detail0, gl_TexCoord[1].st);
    vec4 detail1 = texture2D(tex_detail1, gl_TexCoord[1].st);
    vec4 detail2 = texture2D(tex_detail2, gl_TexCoord[1].st);
    vec4 detail3 = texture2D(tex_detail3, gl_TexCoord[1].st);
    vec4 detail4 = texture2D(tex_detail4, gl_TexCoord[1].st);
    
    gl_FragColor = (layout.r * detail0 +
                    layout.g * detail1 +
                    layout.b * detail2 +
                    (1.0 - layout.r - layout.g - layout.b) * detail3 +
                    (1.0 - layout.a) * detail4)
                    * min(1.0, 0.2 + dot(lightdir2, normal)); // 0.2 is the ambient light.
}
