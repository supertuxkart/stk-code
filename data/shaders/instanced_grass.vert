uniform vec3 windDir;
uniform mat4 ViewProjectionMatrix;
uniform mat4 InverseViewMatrix;

in vec3 Origin;
in vec3 Orientation;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;

out vec3 nor;
out vec2 uv;

mat4 getMatrixFromRotation(vec3 rotation)
{

    // from irrlicht
    float cr = cos( rotation.z );
    float sr = sin( rotation.z );
    float cp = cos( rotation.x );
    float sp = sin( rotation.x );
    float cy = cos( rotation.y );
    float sy = sin( rotation.y );

    float srsp = sr*sp;
    float crsp = cr*sp;

    return mat4(
        vec4(cp * cy, srsp * cy - cr * sy, crsp * cy + sr * sy, 0.),
        vec4(cp * sy, srsp * sy + cr * cy, crsp * sy - sr * cy, 0.),
        vec4(-sp, sr * cp, cr * cp, 0.),
        vec4(0., 0., 0., 1.));
}

mat4 getWorldMatrix(vec3 translation, vec3 rotation)
{
    mat4 result = getMatrixFromRotation(rotation);
    // translation
    result[3].xyz += translation;
    return result;
}

mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation)
{
    mat4 result = transpose(getMatrixFromRotation(rotation));
    // FIXME: it's wrong but the fourth column is not used
    result[3].xyz -= translation;
    return result;
}

void main()
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation) * InverseViewMatrix);
    gl_Position = ViewProjectionMatrix *  ModelMatrix * vec4(Position + windDir * Color.r, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
}
