// Using numerical value from here
// http://content.gpwiki.org/index.php/D3DBook:High-Dynamic_Range_Rendering

vec3 getRGBFromCIEXxy(vec3 YxyColor)
{
    float Yovery = YxyColor.x / max(YxyColor.z, 0.1);
    vec3 XYZ = vec3(YxyColor.y * Yovery, YxyColor.x, (1. - YxyColor.y - YxyColor.z) * Yovery);

    mat3 XYZ2RGB = transpose(mat3(
        vec3(3.2405, -.9693, .0556),
        vec3(-1.5371, 1.8760, -.2040),
        vec3(-.4985, .0416, 1.0572)));

    vec3 RGBColor = XYZ2RGB * XYZ;
    return max(RGBColor, vec3(0.));
}

