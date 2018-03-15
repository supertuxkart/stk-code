in vec3 Position;
in float Energy;
in vec3 Color;
in float Radius;

flat out vec3 center;
flat out float energy;
flat out vec3 col;
flat out float radius;

const float zNear = 1.;

// Code borrowed from https://software.intel.com/en-us/articles/deferred-rendering-for-current-and-future-rendering-pipelines
// Maths explanations are found here http://www.gamasutra.com/view/feature/131351/the_mechanics_of_robust_stencil_.php?page=6

vec2 UpdateClipRegionRoot(float nc,          /* Tangent plane x/y normal coordinate (view space) */
                          float lc,          /* Light x/y coordinate (view space) */
                          float lz,          /* Light z coordinate (view space) */
                          float lightRadius,
                          float cameraScale  /* Project scale for coordinate (_11 or _22 for x/y respectively) */)
{
    float nz = (lightRadius - nc * lc) / lz;
    float pz = (lc * lc + lz * lz - lightRadius * lightRadius) /
               (lz - (nz / nc) * lc);

    if (pz > 0.) {
        float c = -nz * cameraScale / nc;
        if (nc > 0.) // Left side boundary
            return vec2(c, 1.);
        else // Right side boundary
            return vec2(-1., c);
    }
    return vec2(-1., 1.);
}

vec2 UpdateClipRegion(float lc,          /* Light x/y coordinate (view space) */
                      float lz,          /* Light z coordinate (view space) */
                      float lightRadius,
                      float cameraScale  /* Project scale for coordinate (_11 or _22 for x/y respectively) */)
{
    float rSq = lightRadius * lightRadius;
    float lcSqPluslzSq = lc * lc + lz * lz;
    float d = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

    // The camera is inside lignt bounding sphere, quad fits whole screen
    if (d <= 0.)
        return vec2(-1., 1.);

    float a = lightRadius * lc;
    float b = sqrt(d);
    float nx0 = (a + b) / lcSqPluslzSq;
    float nx1 = (a - b) / lcSqPluslzSq;

    vec2 clip0 = UpdateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale);
    vec2 clip1 = UpdateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale);
    return vec2(max(clip0.x, clip1.x), min(clip0.y, clip1.y));
}

// Returns bounding box [min.x, max.x, min.y, max.y] in clip [-1, 1] space.
vec4 ComputeClipRegion(vec3 lightPosView, float lightRadius)
{
    if (lightPosView.z + lightRadius >= zNear) {
        vec2 clipX = UpdateClipRegion(lightPosView.x, lightPosView.z, lightRadius, u_projection_matrix[0][0]);
        vec2 clipY = UpdateClipRegion(lightPosView.y, lightPosView.z, lightRadius, u_projection_matrix[1][1]);

        return vec4(clipX, clipY);
    }

    return vec4(0.);
}


void main(void)
{
    vec4 Center = u_view_matrix * vec4(Position, 1.);
    Center /= Center.w;

    vec2 ProjectedCornerPosition;
    vec4 clip = ComputeClipRegion(Center.xyz, Radius);
    switch (gl_VertexID)
    {
    case 0:
        ProjectedCornerPosition = clip.xz;
        break;
    case 1:
        ProjectedCornerPosition = clip.xw;
        break;
    case 2:
        ProjectedCornerPosition = clip.yz;
        break;
    case 3:
        ProjectedCornerPosition = clip.yw;
        break;
    }

    // Work out nearest depth for quad Z
    // Clamp to near plane in case this light intersects the near plane... don't want our quad to be clipped
    float quadDepth = max(zNear, Center.z - Radius);

    // Project quad depth into clip space
    vec4 quadClip = u_projection_matrix * vec4(0., 0., quadDepth, 1.0f);
    gl_Position = vec4(ProjectedCornerPosition, quadClip.z / quadClip.w, 1.);

    col = Color;
    center = Position;
    energy = Energy;
    radius = Radius;
}
