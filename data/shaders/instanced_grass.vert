uniform vec3 windDir;
uniform mat4 ViewProjectionMatrix;
uniform mat4 ViewMatrix;

in vec3 Origin;
in vec3 Orientation;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;

out vec3 nor;
out vec2 uv;

mat4 getWorldMatrix(vec3 origin, vec3 rotation)
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

    // rotation part
    mat4 result = mat4(
        vec4(cp * cy, srsp * cy - cr * sy, crsp * cy + sr * sy, 0.),
        vec4(cp * sy, srsp * sy + cr * cy, crsp * sy - sr * cy, 0.),
        vec4(-sp, sr * cp, cr * cp, 0.),
        vec4(0., 0., 0., 1.));
    // translation
    result[3].xyz += origin;
    return result;

/*		M[0] = (T)( cp*cy );
		M[1] = (T)( cp*sy );
		M[2] = (T)( -sp );

		const f64 srsp = sr*sp;
		const f64 crsp = cr*sp;

		M[4] = (T)( srsp*cy-cr*sy );
		M[5] = (T)( srsp*sy+cr*cy );
		M[6] = (T)( sr*cp );

		M[8] = (T)( crsp*cy+sr*sy );
		M[9] = (T)( crsp*sy-sr*cy );
		M[10] = (T)( cr*cp );*/
}


void main()
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation);
    mat4 TransposeInverseModelView = transpose(inverse(ViewMatrix * ModelMatrix));
    gl_Position = ViewProjectionMatrix *  ModelMatrix * vec4(Position + windDir * Color.r, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = Texcoord;
}
