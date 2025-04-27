// Push constants to pass face index, dimensions, and sample count.
layout(push_constant) uniform PushConstants {
    int size;            // width and height for current mipmap level
    int sampleCount;     // number of samples for integration
    int mipmapLevel;     // current mipmap level
    int mipmapCount;     // total mipmap levels
} pc;

// Returns the radical inverse of "bits" with base 2.
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

// Generate a 2D Hammersley sequence value.
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// Converts face index and UV coordinates in [0,1] to a normalized direction vector.
vec3 FaceUVtoDir(int face, vec2 uv)
{
    // Map UV from [0, 1] to [-1, 1]
    uv = uv * 2.0 - 1.0;
    vec3 dir;
    if (face == 0)        // +X
        dir = vec3(1.0, -uv.y, -uv.x);
    else if (face == 1)   // -X
        dir = vec3(-1.0, -uv.y, uv.x);
    else if (face == 2)   // +Y
        dir = vec3(uv.x, 1.0, uv.y);
    else if (face == 3)   // -Y
        dir = vec3(uv.x, -1.0, -uv.y);
    else if (face == 4)   // +Z
        dir = vec3(uv.x, -uv.y, 1.0);
    else if (face == 5)   // -Z
        dir = vec3(-uv.x, -uv.y, -1.0);
    return normalize(dir);
}

const float PI = 3.14159265359;
