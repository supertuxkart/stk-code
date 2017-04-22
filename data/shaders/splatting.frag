#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex_layout;
layout(bindless_sampler) uniform sampler2D tex_detail0;
layout(bindless_sampler) uniform sampler2D tex_detail1;
layout(bindless_sampler) uniform sampler2D tex_detail2;
layout(bindless_sampler) uniform sampler2D tex_detail3;
#else
uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
#endif

in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main() {
    // Splatting part
    vec4 splatting = texture(tex_layout, uv_bis);
    vec4 detail0 = texture(tex_detail0, uv);
    vec4 detail1 = texture(tex_detail1, uv);
    vec4 detail2 = texture(tex_detail2, uv);
    vec4 detail3 = texture(tex_detail3, uv);
    vec4 detail4 = vec4(0.0);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    detail0.xyz = pow(detail0.xyz, vec3(2.2));
    detail1.xyz = pow(detail1.xyz, vec3(2.2));
    detail2.xyz = pow(detail2.xyz, vec3(2.2));
    detail3.xyz = pow(detail3.xyz, vec3(2.2));
#endif
#endif

    vec4 splatted = splatting.r * detail0 +
        splatting.g * detail1 +
        splatting.b * detail2 +
        max(0., (1.0 - splatting.r - splatting.g - splatting.b)) * detail3;

    FragColor = vec4(getLightFactor(splatted.xyz, vec3(1.), 0., 0.), 1.);
}
