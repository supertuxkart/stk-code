// Using numerical value from here
// http://content.gpwiki.org/index.php/D3DBook:High-Dynamic_Range_Rendering

vec3 getCIEYxy(vec3 rgbColor)
{
    // convert rgb to srgb
    vec3 sRGBColor = rgbColor;//vec3(pow(rgbColor.x, 1. / 2.2), pow(rgbColor.y, 1. / 2.2), pow(rgbColor.z, 1. / 2.2));

    mat3 sRGB2XYZ = transpose(mat3(
        vec3(.4125, .2126, .0193),
        vec3(.3576, .7152, .1192),
        vec3(.1805, .0722, .9505)));

    vec3 XYZ = sRGB2XYZ * sRGBColor;
    float sum = dot(vec3(1.), XYZ);
    return vec3(XYZ.y, XYZ.xy / sum);
}