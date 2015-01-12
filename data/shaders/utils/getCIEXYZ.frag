// Using numerical value from here
// http://content.gpwiki.org/index.php/D3DBook:High-Dynamic_Range_Rendering

vec3 getCIEYxy(vec3 rgbColor)
{
    mat3 RGB2XYZ = transpose(mat3(
        vec3(.4125, .2126, .0193),
        vec3(.3576, .7152, .1192),
        vec3(.1805, .0722, .9505)));

    vec3 xYz = RGB2XYZ * rgbColor;
    float tmp = max(xYz.x + xYz.y + xYz.z, 0.1);
    return vec3(xYz.y, xYz.xy / tmp);
}
