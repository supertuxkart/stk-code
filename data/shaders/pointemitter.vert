#version 130
uniform int dt;
uniform mat4 sourcematrix;
uniform int level;
uniform float size_increase_factor;

in vec3 particle_position_initial;
in float lifetime_initial;
in vec3 particle_velocity_initial;
in float size_initial;

in vec3 particle_position;
in float lifetime;
in vec3 particle_velocity;
in float size;

out vec3 new_particle_position;
out float new_lifetime;
out vec3 new_particle_velocity;
out float new_size;

void main(void)
{
  float updated_lifetime = lifetime  + (float(dt)/lifetime_initial);
  if (updated_lifetime > 1.)
  {
    if (gl_VertexID <= level)
    {
      float dt_from_last_frame = fract(updated_lifetime) * lifetime_initial;
      vec4 updated_initialposition = sourcematrix * vec4(particle_position_initial, 1.0);
      vec4 updated_initial_velocity = sourcematrix * vec4(particle_position_initial + particle_velocity_initial, 1.0) - updated_initialposition;
      new_particle_position = updated_initialposition.xyz + updated_initial_velocity.xyz * float(dt_from_last_frame);
      new_particle_velocity = updated_initial_velocity.xyz;
      new_lifetime = fract(updated_lifetime);
      new_size = mix(size_initial, size_initial * size_increase_factor, fract(updated_lifetime));
    }
    else
    {
        new_lifetime = fract(updated_lifetime);
        new_size = 0;
    }
  }
  else
  {
    new_particle_position = particle_position + particle_velocity.xyz * float(dt);
    new_particle_velocity = particle_velocity;
    new_lifetime = updated_lifetime;
    new_size = (size == 0) ? 0. : mix(size_initial, size_initial * size_increase_factor, updated_lifetime);
  }
  gl_Position = vec4(0.);
}
