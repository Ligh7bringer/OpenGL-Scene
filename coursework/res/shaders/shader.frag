#version 440

#ifndef POINT
#define POINT
// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};
#endif

#ifndef SPOT
#define SPOT
// Spot light data
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
#endif

#ifndef MATERIAL
#define MATERIAL
// Material data
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};
#endif

vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour);
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                    in vec4 tex_colour);
vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord);

//lights
uniform spot_light spot;
uniform point_light points[2];
// Material of the object being rendered
uniform material mat;
// Position of the eye
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;
// Normal map to sample from
uniform sampler2D normal_map;

// Incoming vertex position
layout(location = 0) in vec3 position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming tangent
layout(location = 3) in vec3 tangent;
// Incoming binormal
layout(location = 4) in vec3 binormal;

// Outgoing colour
layout(location = 0) out vec4 colour;


void main() {

  colour = vec4(0.0, 0.0, 0.0, 1.0);
  // *********************************
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);
  vec3 n = calc_normal(normal, tangent, binormal, normal_map, tex_coord);
  // Sum point lights
  for(int i = 0; i < 2; i++) {
	   colour += calculate_point(points[i], mat, position, n, view_dir, tex_colour);
  }
  // Sum spot lights
	colour += calculate_spot(spot, mat, position, n, view_dir, tex_colour);
  colour.a = 1.0;
  // *********************************
}
