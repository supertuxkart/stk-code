vec4 getVertexColor(uint packed)
{
    vec4 vertex_color;
    vertex_color.a = float(packed >> 24) / 255.0;
    vertex_color.r = float((packed >> 16) & 0xff) / 255.0;
    vertex_color.g = float((packed >> 8) & 0xff) / 255.0;
    vertex_color.b = float(packed & 0xff) / 255.0;
    return vertex_color;
}
