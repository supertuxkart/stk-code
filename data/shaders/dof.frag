uniform sampler2D tex;
uniform sampler2D dtex;

layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

in vec2 uv;
out vec4 FragColor;

float focalDepth = 10.;
float maxblur = 1.;
float range = 100.;

void main()
{
    float curdepth = texture(dtex, uv).x;
    vec4 FragPos = InverseProjectionMatrix * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
    FragPos /= FragPos.w;

    float depth = FragPos.z;
    float blur = clamp(abs(depth - focalDepth) / range, -maxblur, maxblur);

    vec2 offset = 10. / screen;

    vec4 col = texture(tex, uv);
    vec4 colOriginal = col;
    // Weight from here http://artmartinsh.blogspot.fr/2010/02/glsl-lens-blur-filter-with-bokeh.html

    col += texture(tex, uv + (vec2(0.0, 0.4) * offset) * blur);
    col += texture(tex, uv + (vec2(0.15, 0.37) * offset) * blur);
    col += texture(tex, uv + (vec2(0.29,0.29) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.37,0.15) * offset) * blur);
    col += texture(tex, uv + (vec2(0.4, 0.0) * offset) * blur);
    col += texture(tex, uv + (vec2(0.37, -0.15) * offset) * blur);
    col += texture(tex, uv + (vec2(0.29, -0.29) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.15, -0.37) * offset) * blur);
    col += texture(tex, uv + (vec2(0.0, -0.4) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.15, 0.37) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.29, 0.29) * offset) * blur);
    col += texture(tex, uv + (vec2(0.37, 0.15) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.4, 0.0) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.37, -0.15) * offset) * blur);
    col += texture(tex, uv + (vec2(-0.29, -0.29) * offset) * blur);
    col += texture(tex, uv + (vec2(0.15, -0.37) * offset) * blur);

    col += texture(tex, uv + (vec2(0.15, 0.37) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(-0.37, 0.15) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(0.37, -0.15) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(-0.15, -0.37) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(-0.15, 0.37) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(0.37, 0.15) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(-0.37, -0.15) * offset) * blur * 0.9);
    col += texture(tex, uv + (vec2(0.15, -0.37) * offset) * blur * 0.9);

    col += texture(tex, uv + (vec2(0.29, 0.29) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(0.4, 0.0) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(0.29, -0.29) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(0.0, -0.4) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(-0.29, 0.29) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(-0.4, 0.0) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(-0.29, -0.29) * offset) * blur * 0.7);
    col += texture(tex, uv + (vec2(0.0, 0.4) * offset) * blur  *0.7);

    col += texture(tex, uv + (vec2(0.29, 0.29) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(0.4, 0.0) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(0.29, -0.29) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(0.0, -0.4) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(-0.29, 0.29) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(-0.4, 0.0) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(-0.29, -0.29) * offset) * blur * 0.4);
    col += texture(tex, uv + (vec2(0.0, 0.4) * offset) * blur * 0.4);
    
    col = vec4(col.rgb / 41.0, col.a);
    depth = clamp((FragPos.z/280), 0., 1.);
    depth  = (1 - depth);
    vec3 final = colOriginal.rgb * depth + col.rgb * (1 - depth);

    FragColor = vec4(final, 1.);
}
