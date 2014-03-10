uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

in vec3 Position;
in float Energy;
in vec3 Color;

in vec2 Corner;

flat out vec3 center;
flat out float energy;
flat out vec3 col;

const float zNear = 1.;

void main(void)
{
    // Beyond that value, light is too attenuated
    float r = 40 * Energy;
    center = Position;
    energy = Energy;
    vec4 Center = ViewMatrix * vec4(Position, 1.);
    if (Center.z > zNear) // Light is in front of the cam
    {
        vec3 UnitCenter = normalize(-Center.xyz);
        float clampedR = min(r, Center.z - 1.);
        float cosTheta = dot(UnitCenter, vec3(0., 0., -1));
        float d = clampedR / cosTheta;
        Center.xyz += d * UnitCenter;
    }
    else if (Center.z + r > zNear) // Light is behind the cam but in range
    {
        Center.z = zNear;
        // TODO: Change r so that we make the screen aligned quad fits light range.
    }
    col = Color;
    gl_Position = ProjectionMatrix * (Center + r * vec4(Corner, 0., 0.));
}
