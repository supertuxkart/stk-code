#version 130
uniform int dt;
uniform vec3 source;
uniform int duration;

in vec3 particle_position;
in float lifetime;
in vec4 particle_velocity;

out vec3 new_particle_position;
out float new_lifetime;
out vec4 new_particle_velocity;

void main(void)
{
  new_particle_position = (lifetime > 0.) ? particle_position + particle_velocity.xyz * float(dt) : vec3(0., 0., 0.);
  new_lifetime = (lifetime > 0.) ? lifetime - float(dt) : float(duration);
  new_particle_velocity = particle_velocity;
  gl_Position = vec4(0.);
}
