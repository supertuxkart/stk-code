

uniform sampler2D texture;
varying vec3 normal;
uniform vec3 lightdir;
varying vec4 vertex_color;
varying vec3 eyeVec;
varying vec3 lightVec;

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
    //gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    // specular (phong)
    vec3 R = normalize(reflect(lightVec, normal));
    float specular = max(dot(R,eyeVec),0.0);
    
    //gl_FragColor = vec4(specular, specular, specular, 1.0);
    
    if (specular > 0.0)
    {
        // weak specular
        specular = specular*specular;
        specular = specular*specular;
        float specular_weak = specular*2.0; //max(specular*1.1, 1.0);
        gl_FragColor += vec4(specular_weak, specular_weak, specular_weak, 0.0);
        
        /*
        // strong specular
        specular = specular*specular;
        float specular_strong = specular;
        gl_FragColor += vec4(specular_strong, specular_strong, specular_strong, 0.0);
        */
    }
}
