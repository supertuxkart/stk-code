layout (input_attachment_index = 0, binding = 0) uniform subpassInput u_hdr;

layout(location = 0) out vec4 o_color;

#include "utils/constants_utils.glsl"

void main()
{
    o_color = vec4(convertColor(subpassLoad(u_hdr).xyz), 1.0);
}
