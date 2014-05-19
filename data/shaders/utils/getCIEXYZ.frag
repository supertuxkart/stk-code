// Using numerical value from here
// http://content.gpwiki.org/index.php/D3DBook:High-Dynamic_Range_Rendering

vec3 getCIEYxy(vec3 rgbColor)
{
    mat3 sRGB2XYZ = transpose(mat3(
        vec3(.4125, .2126, .0193),
        vec3(.3576, .7152, .1192),
        vec3(.1805, .0722, .9505)));

    return sRGB2XYZ * rgbColor;
}
