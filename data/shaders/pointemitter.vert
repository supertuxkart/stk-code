#version 130
uniform int dt;
uniform mat4 sourcematrix;
uniform mat4 tinvsourcematrix;

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
  vec4 initialposition = sourcematrix * vec4(particle_position_initial, 1.0);
  vec4 adjusted_initial_velocity = tinvsourcematrix * vec4(particle_velocity_initial, 1.0);
  adjusted_initial_velocity /= adjusted_initial_velocity.w;
  float adjusted_lifetime = lifetime  + (float(dt)/lifetime_initial);
  new_particle_position = (adjusted_lifetime < 1.) ? particle_position + particle_velocity.xyz * float(dt) : initialposition.xyz;
  new_lifetime = (adjusted_lifetime < 1.) ? adjusted_lifetime : 0.;
  new_particle_velocity = (adjusted_lifetime < 1.) ? particle_velocity : adjusted_initial_velocity.xyz;
  new_size = size_initial;
  gl_Position = vec4(0.);
}
