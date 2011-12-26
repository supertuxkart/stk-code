// Creates a bubble (wave) effect by distorting the texture depending on time

uniform float time;
varying vec2 uv;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
    
    float delta_x = cos(time*3.0) * sin( 4.0 * gl_TexCoord[0].st.s * 6.28318531 );
    float delta_y = cos(time*2.0) * sin( 3.0 * gl_TexCoord[0].st.t * 6.28318531 );

    uv = gl_TexCoord[0].st + vec2(0.02*delta_x, 0.02*delta_y);
}
