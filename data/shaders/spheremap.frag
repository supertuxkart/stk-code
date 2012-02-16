

uniform sampler2D texture;
varying vec3 normal;
uniform vec3 lightdir;
varying vec4 vertex_color;

void main()
{
    vec3 forward = vec3(0.0, 0.0, 1.0);
    
    // get the angle between the forward vector and the horizontal portion of the normal
    vec3 normal_x = normalize(vec3(normal.x, 0.0, normal.z));
    float sin_theta_x = length(cross( forward, normal_x )) * normal.x/abs(normal.x);
    
    // get the angle between the forward vector and the vertical portion of the normal
    vec3 normal_y = normalize(vec3(0.0, normal.y, normal.z));
    float sin_theta_y = length(cross( forward, normal_y ))* normal.y/abs(normal.y);
    
    vec4 detail0 = texture2D(texture, vec2(0.5 + sin_theta_x*0.5, 0.5 + sin_theta_y*0.5));

    gl_FragColor = detail0 * (0.5 + dot(lightdir, normal)) * vertex_color; // 0.5 is the ambient light.
    //gl_FragColor = vec4(sin_theta_y*0.5 + 0.5, 0.0, 1.0 - (sin_theta_y*0.5 + 0.5), 1.0);
}
