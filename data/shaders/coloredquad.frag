uniform ivec4 color;

#if __VERSION__ >= 130
out vec4 FragColor;
#else
#define FragColor gl_FragColor
#endif


void main()
{
	FragColor = vec4(color) / 255.;
}
