uniform int dt;
uniform mat4 sourcematrix;
uniform int level;
uniform float size_increase_factor;

uniform float track_x;
uniform float track_z;
uniform float track_x_len;
uniform float track_z_len;
uniform samplerBuffer heightmap;

layout (location = 4) in vec3 particle_position_initial;
layout (location = 5) in float lifetime_initial;
layout (location = 6) in vec3 particle_velocity_initial;
layout (location = 7) in float size_initial;

layout (location = 0) in vec3 particle_position;
layout (location = 1) in float lifetime;
layout (location = 2) in vec3 particle_velocity;
layout (location = 3) in float size;

out vec3 new_particle_position;
out float new_lifetime;
out vec3 new_particle_velocity;
out float new_size;

void main(void)
{
  bool reset = false;

  float i_as_float = clamp(256. * (particle_position.x - track_x) / track_x_len, 0., 255.);
  float j_as_float = clamp(256. * (particle_position.z - track_z) / track_z_len, 0., 255.);
  int i = int(i_as_float);
  int j = int(j_as_float);

  float h = particle_position.y - texelFetch(heightmap, i * 256 + j).r;
  reset = h < 0.;

  vec4 initialposition = sourcematrix * vec4(particle_position_initial, 1.0);
  vec4 adjusted_initial_velocity = sourcematrix * vec4(particle_position_initial + particle_velocity_initial, 1.0) - initialposition;
  float adjusted_lifetime = lifetime  + (float(dt)/lifetime_initial);
  reset = reset || (adjusted_lifetime > 1.) && (gl_VertexID <= level);
  reset = reset || (lifetime < 0.);
  new_particle_position = !reset ? particle_position + particle_velocity.xyz * float(dt) : initialposition.xyz;
  new_lifetime = !reset ? adjusted_lifetime : 0.;
  new_particle_velocity = !reset ? particle_velocity : adjusted_initial_velocity.xyz;
  new_size = !reset ? mix(size_initial, size_initial * size_increase_factor, adjusted_lifetime) : size_initial;
  gl_Position = vec4(0.);
}
