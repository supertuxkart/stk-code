uniform int dt;
uniform mat4 previous_frame_sourcematrix;
uniform mat4 sourcematrix;
uniform int level;
uniform float size_increase_factor;

#if __VERSION__ >= 330
layout (location = 4) in vec3 particle_position_initial;
layout (location = 5) in float lifetime_initial;
layout (location = 6) in vec3 particle_velocity_initial;
layout (location = 7) in float size_initial;

layout (location = 0) in vec3 particle_position;
layout (location = 1) in float lifetime;
layout (location = 2) in vec3 particle_velocity;
layout (location = 3) in float size;
#else
in vec3 particle_position_initial;
in float lifetime_initial;
in vec3 particle_velocity_initial;
in float size_initial;

in vec3 particle_position;
in float lifetime;
in vec3 particle_velocity;
in float size;
#endif

out vec3 new_particle_position;
out float new_lifetime;
out vec3 new_particle_velocity;
out float new_size;

void main(void)
{
    float updated_lifetime = lifetime  + (float(dt)/lifetime_initial);
    if (updated_lifetime > 1.)
    {
        if (gl_VertexID < level)
        {
            float dt_from_last_frame = fract(updated_lifetime) * lifetime_initial;
            float coeff = dt_from_last_frame / float(dt);
            
            vec4 previous_frame_position = previous_frame_sourcematrix * vec4(particle_position_initial, 1.0);
            vec4 current_frame_position  = sourcematrix * vec4(particle_position_initial, 1.0);
            
            vec4 updated_initialposition = mix(current_frame_position,
                                               previous_frame_position,
                                               coeff);
                                                
            vec4 updated_initial_velocity = mix(sourcematrix * vec4(particle_velocity_initial, 0.0),
                                                previous_frame_sourcematrix * vec4(particle_velocity_initial, 0.0),
                                                coeff);
                                          //+ (current_frame_position - previous_frame_position) / dt;
            //To be accurate, emitter speed should be added.
            //But the simple formula ( (current_frame_position - previous_frame_position) / dt ) with a constant speed
            //between 2 frames creates visual artifacts when the framerate is low, and a more accurate formula would need
            //more complex computations.
                                                
            new_particle_position = updated_initialposition.xyz + dt_from_last_frame * updated_initial_velocity.xyz;
            new_particle_velocity = updated_initial_velocity.xyz;
            
            new_lifetime = fract(updated_lifetime);
            new_size = mix(size_initial, size_initial * size_increase_factor, fract(updated_lifetime));                                                                       
        }
        else
        {
            new_lifetime = fract(updated_lifetime);
            new_size = 0.0;
        }
    }
    else
    {
        new_particle_position = particle_position + particle_velocity.xyz * float(dt);
        new_particle_velocity = particle_velocity;
        new_lifetime = updated_lifetime;
        new_size = (size == 0.0) ? 0. : mix(size_initial, size_initial * size_increase_factor, updated_lifetime);
    }
    gl_Position = vec4(0.);
}
