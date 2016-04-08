const float DEG2GRAD = 3.14 / 180.;


mat4 getMatrixFromRotation(vec3 rotation)
{

    // from irrlicht
    float cr = cos(DEG2GRAD * rotation.x );
    float sr = sin(DEG2GRAD * rotation.x );
    float cp = cos(DEG2GRAD * rotation.y );
    float sp = sin(DEG2GRAD * rotation.y );
    float cy = cos(DEG2GRAD * rotation.z );
    float sy = sin(DEG2GRAD * rotation.z );

    float srsp = sr*sp;
    float crsp = cr*sp;

    return transpose(mat4(
        vec4(cp * cy, srsp * cy - cr * sy, crsp * cy + sr * sy, 0.),
        vec4(cp * sy, srsp * sy + cr * cy, crsp * sy - sr * cy, 0.),
        vec4(-sp, sr * cp, cr * cp, 0.),
        vec4(0., 0., 0., 1.)));
}

mat4 getScaleMatrix(vec3 scale)
{
    mat4 result = mat4(
		vec4(scale.x, 0., 0., 0.),
		vec4(0., scale.y, 0., 0.),
		vec4(0., 0., scale.z, 0.),
		vec4(0., 0., 0., 1.)
);
    return result;
}

mat4 getWorldMatrix(vec3 translation, vec3 rotation, vec3 scale)
{
    mat4 result = getMatrixFromRotation(rotation);
    // translation
    result[3].xyz += translation;
    return result * getScaleMatrix(scale);
}

mat4 getInverseWorldMatrix(vec3 translation, vec3 rotation, vec3 scale)
{
    mat4 result = transpose(getMatrixFromRotation(rotation));
    // FIXME: it's wrong but the fourth column is not used
    // result[3].xyz -= translation;
    return getScaleMatrix(1. / scale) * result;
}