layout(location = 1) in vec2 f_uv;
layout(location = 5) in vec3 f_normal;
layout(location = 8) in vec4 f_world_position;

layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec2 o_displace_mask;
layout(location = 1) out vec4 o_displace_ssr;

layout(push_constant) uniform Constants
{
    vec4 m_displace_direction;
} u_push_constants;

#include "utils/camera.glsl"
#include "utils/constants_utils.glsl"
#include "utils/sample_mesh_texture.glsl"
#include "../utils/displace_utils.frag"
#include "../utils/screen_space_reflection.frag"

layout (set = 2, binding = 2) uniform samplerCube u_skybox_texture;
layout (set = 3, binding = 0) uniform sampler2D u_displace_color;
layout (set = 3, binding = 1) uniform sampler2DShadow u_depth;
layout (set = 3, binding = 2) uniform sampler2D u_hiz_depth;

#ifdef PBR_ENABLED

// Start tracing in this level.
#define HIZ_START_LEVEL      0
// Stop tracing if current level is higher than this. (higher level means lower value)
#define HIZ_STOP_LEVEL       0
#define HIZ_MAX_LEVEL        6

// Set to 1 to disable HiZ and perform naive linear search.
#define DEBUG_LINEAR_SEARCH  0
#define MAX_THICKNESS        0.001

vec3 intersectDepthPlane(vec3 o, vec3 d, float z)
{
    return o + d * z;
}

// Index of the cell that contains the given 2D position.
ivec2 getCell(vec2 screenUV, ivec2 cellCount)
{
    return ivec2(screenUV * cellCount);
}

// The number of cells in the quad tree at the given level.
ivec2 getCellCount(int level)
{
    return textureSize(u_hiz_depth, level);
}

// Returns screen space position of the intersection
// between o + d*t and the closest cell boundary at current HiZ level.
vec3 intersectCellBoundary(
    vec3 pos, vec3 dir,
    ivec2 cell, ivec2 cellCount,
    vec2 crossStep, vec2 crossOffset)
{
    vec3 intersection = vec3(0.0);

    vec2 index = cell + crossStep;
    vec2 boundary = index / vec2(cellCount); // Screen space position of the boundary
    boundary += crossOffset;

    vec2 delta = boundary - pos.xy;
    delta /= dir.xy;
    float t = min(delta.x, delta.y);

    intersection = intersectDepthPlane(pos, dir, t);
    return intersection;
}

bool crossedCellBoundary(ivec2 oldCellIx, ivec2 newCellIx)
{
    return any(notEqual(oldCellIx, newCellIx));
}

// Minimum depth of the current cell in the current HiZ level.
float getMinDepthPlane(ivec2 cellIx, int level)
{
    return texelFetch(u_hiz_depth, cellIx, level).x;
}

float getMaxTraceDistance(vec3 p, vec3 v)
{
    vec3 traceDistances;
    if (v.x < 0.0)
        traceDistances.x = p.x / (-v.x);
    else
        traceDistances.x = (1.0 - p.x) / v.x;

    if (v.y < 0.0)
        traceDistances.y = p.y / (-v.y);
    else
        traceDistances.y = (1.0 - p.y) / v.y;

    if (v.z < 0.0)
        traceDistances.z = p.z / (-v.z);
    else
        traceDistances.z = (1.0 - p.z) / v.z;

    return min(traceDistances.x, min(traceDistances.y, traceDistances.z));
}

// p            : Screen space position
// v            : Screen space reflection direction
// hitPointSS   : Returns screen space hit point
// Return value : Whether RT actually hit a surface
bool traceHiZ(vec3 p, vec3 v, out vec2 hitPointSS)
{
    const int maxLevel = min(HIZ_MAX_LEVEL, textureQueryLevels(u_hiz_depth) - 1); // Last mip level
    float maxTraceDistance = getMaxTraceDistance(p, v);

    // Get the cell cross direction and a small offset to enter
    // the next cell when doing cell crossing.
    vec2 crossStep = vec2(v.x >= 0 ? 1 : -1, v.y >= 0 ? 1 : -1);
    vec2 crossOffset = crossStep / u_camera.m_viewport.zw / 128.;
    crossStep = clamp(crossStep, 0.0, 1.0);

    // Set current ray to the original screen coordinate and depth.
    vec3 ray = p;
    float minZ = ray.z;
    float maxZ = ray.z + v.z * maxTraceDistance;
    float deltaZ = maxZ - minZ;

    vec3 o = ray;
    vec3 d = v * maxTraceDistance;

    int level = HIZ_START_LEVEL;
    int deepestLevel = level;
#if DEBUG_LINEAR_SEARCH
    level = 0;
#endif
    uint iterations = 0;
    bool isBackwardRay = v.z < 0;
    float rayDir = isBackwardRay ? -1.0 : 1.0;

    // Cross to next cell s.t. we don't get a self-intersection immediately.
    ivec2 startCellCount = getCellCount(level);
    ivec2 rayCell = getCell(ray.xy, startCellCount);
    ray = intersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset * 64.);

    while (level >= HIZ_STOP_LEVEL && ray.z * rayDir <= maxZ * rayDir &&
        iterations < u_hiz_iterations)
    {
        // Get the cell number of our current ray.
        ivec2 cellCount = getCellCount(level);
        ivec2 oldCellIx = getCell(ray.xy, cellCount);

        // Get the minimum depth plane in which the current ray resides.
        float cellMinZ = getMinDepthPlane(oldCellIx, level);

        // Intersect only if ray depth is below the minimum depth plane.
        vec3 tempRay;
        if (cellMinZ > ray.z && !isBackwardRay)
            tempRay = intersectDepthPlane(o, d, (cellMinZ - minZ) / deltaZ);
        else
            tempRay = ray;

        ivec2 newCellIx = getCell(tempRay.xy, cellCount);
        float thickness = level == 0 ? (ray.z - cellMinZ) : 0;

        bool crossed = (isBackwardRay && (cellMinZ > ray.z))
                    || (thickness > MAX_THICKNESS) || crossedCellBoundary(oldCellIx, newCellIx);

        if (crossed)
        {
            ray = intersectCellBoundary(o, d, oldCellIx, cellCount, crossStep, crossOffset);
            level = min(maxLevel, level + 1);
            deepestLevel = max(deepestLevel, level);
#if DEBUG_LINEAR_SEARCH
            level = 0;
#endif
        }
        else
        {
            ray = tempRay;
            level = level - 1;
        }

        iterations += 1;
    }

    // Results
    //debugDeepestLevel = deepestLevel;
    //debugIterations = iterations;
    hitPointSS = ray.xy;
    return level < HIZ_STOP_LEVEL && iterations < u_hiz_iterations;
}

#endif

void main()
{
#ifdef PBR_ENABLED
    float horiz = sampleMeshTexture2(f_material_id, f_uv + u_push_constants.m_displace_direction.xy * 150.).x;
    float vert = sampleMeshTexture2(f_material_id, (f_uv.yx + u_push_constants.m_displace_direction.zw * 150.) * vec2(0.9)).x;
    vec2 mask = getDisplaceShift(horiz, vert);
    mask = (mask + 1.0) * 0.5;
    o_displace_mask = mask;
    if (u_ssr)
    {
        float alpha = sampleMeshTexture0(f_material_id, f_uv).a;
        if (alpha == 0.0)
        {
            o_displace_ssr = vec4(0.0);
            return;
        }
        // eye-space position
        vec3 xpos = (u_camera.m_view_matrix * f_world_position).xyz;
        // eye-space view direction (points from surface toward eye at origin)
        vec3 eyedir = -normalize(xpos);
        // eye-space normal
        vec3 normal = (u_camera.m_view_matrix * vec4(normalize(f_normal), 0)).xyz;

        // bail out immediately if normal is facing away from the camera,
        // dot(normal, eyedir) <= 0 means back-facing
        float NdotV = dot(normal, eyedir);
        if (NdotV <= 0.0)
        {
            o_displace_ssr = vec4(0.0);
            return;
        }

        // compute reflection in eye-space
        vec3 reflected = reflect(-eyedir, normal);
        // bring it back into world-space
        vec3 world_reflection = (u_camera.m_inverse_view_matrix *
            vec4(reflected, 0.0)).xyz;

        // fallback to skybox
        vec4 fallback = texture(u_skybox_texture, world_reflection);

        // early exit if normal is facing camera too directly (no meaningful reflection)
        if (normal.z < -0.75)
        {
            o_displace_ssr = fallback;
            return;
        }

        vec4 result;
        vec2 viewport_scale = u_camera.m_viewport.zw / u_camera.m_screensize;
        vec2 viewport_offset = u_camera.m_viewport.xy / u_camera.m_screensize;
        bool hit = true;
        vec2 coords;
        if (u_hiz_iterations == 0)
        {
            coords = RayCast(reflected, xpos, u_camera.m_projection_matrix,
                viewport_scale, viewport_offset, u_depth);
        }
        else
        {
            vec3 positionSS = CalcCoordFromPosition(xpos,
                u_camera.m_projection_matrix, vec2(1.0), vec2(0.0));
            vec3 positionCS = positionSS;
            positionCS.xy = 2.0 * positionCS.xy - 1.0;
            vec3 position2VS = xpos + 1000.0 * reflected;
            vec4 position2CS = u_camera.m_projection_matrix * vec4(position2VS, 1.0);
            position2CS /= position2CS.w;
            vec3 position2SS = position2CS.xyz;
            position2SS.xy = vec2(0.5) + 0.5 * position2SS.xy;
            vec3 reflectionDirSS = normalize(position2SS - positionSS);
            // Trace HiZ to find the hit point.
            hit = traceHiZ(positionSS, reflectionDirSS, coords);
            coords = coords * viewport_scale + viewport_offset;
        }
        vec2 viewport_coords = (coords - viewport_offset) / viewport_scale;
        if (!hit || viewport_coords.x < 0. || viewport_coords.x > 1. ||
            viewport_coords.y < 0. || viewport_coords.y > 1.)
        {
            result = fallback;
        }
        else
        {
            result = texture(u_displace_color, coords);
            float edge = GetEdgeFade(coords, viewport_scale, viewport_offset);
            //float fresnel = pow(1.0 - NdotV, 2.0);
            float fresnel = (1.0 - NdotV) * (1.0 - NdotV);
            float blend_weight = edge * fresnel;
            result = mix(fallback, result, blend_weight);
        }
        o_displace_ssr = result;
    }
#endif
}
