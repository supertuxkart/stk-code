#version 130
uniform int dt;
uniform mat4 sourcematrix;
uniform int duration;

in vec3 particle_position_initial;
in float lifetime_initial;
in vec4 particle_velocity_initial;

in vec3 particle_position;
in float lifetime;
in vec4 particle_velocity;

out vec3 new_particle_position;
out float new_lifetime;
out vec4 new_particle_velocity;

void main(void)
{
  vec4 initialposition = sourcematrix * vec4(particle_position_initial, 1.0);
  new_particle_position = (lifetime < 1.) ? particle_position + particle_velocity.xyz * float(dt) : initialposition.xyz;
  new_lifetime = (lifetime < 1.) ? lifetime  + (float(dt)/lifetime_initial) : 0.;
  new_particle_velocity = (lifetime < 1.) ? particle_velocity : particle_velocity_initial;
  gl_Position = vec4(0.);
}
