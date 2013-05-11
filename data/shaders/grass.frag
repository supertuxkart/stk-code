uniform vec4 fogColor;
uniform float fogFrom;
uniform float fogTo;
uniform int fog;
uniform sampler2D tex;
varying vec4 coord;

void main()
{
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    vec4 solidColor = vec4(color.r, color.g, color.b, 1);
    
    if (fog == 1)
    {
        if (coord.z > fogTo)
        {
            gl_FragColor = fogColor;
        }
        else if (coord.z > fogFrom)
        {
            float fogIntensity = (coord.z - fogFrom) / (fogTo - fogFrom);
            vec4 color2 = fogIntensity*fogColor + (1.0 - fogIntensity)*solidColor;
            color2.a = color.a;
            gl_FragColor = color2;
        }
        else
        {
            gl_FragColor = color;
        }
    }
    else
    {
        gl_FragColor = color;
    }
}
