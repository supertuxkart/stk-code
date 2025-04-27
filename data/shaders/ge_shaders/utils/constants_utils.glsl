layout (constant_id = 0) const bool u_ibl = true;
layout (constant_id = 1) const float u_specular_levels_minus_one = 0.0;

vec3 convertColor(vec3 input_color)
{
    if (u_ibl)
    {
        return (input_color * (6.5 * input_color + 0.45)) /
            (input_color * (5.0 * input_color + 1.75) + 0.05);
    }
    else
    {
        return (input_color * (7.0 * input_color + 0.75)) /
            (input_color * (5.0 * input_color + 1.75) + 0.05);
    }
}
