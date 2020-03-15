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

// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour) {
  // *********************************
  // Get distance between point light and vertex
  float dist = distance(point.position, position);
  // Calculate attenuation factor
  float att_factor = 1/(point.constant + (point.linear*dist) + (point.quadratic*(dist*dist)));
  // Calculate light colour
  vec4 light_colour = att_factor * point.light_colour;
  // Calculate light dir
  vec3 light_dir =  normalize(point.position - position);
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
   vec4 diff_comp = max(dot(normal, light_dir),0.0) * mat.diffuse_reflection * light_colour;
  // Calculate half vector
  vec3 half_vec = normalize(view_dir + light_dir);
  // Calculate specular component
  vec4 spec_comp = pow(max(dot(normal, half_vec),0.0), mat.shininess) * mat.specular_reflection * light_colour;
  // Calculate primary colour component
  vec4 primary = mat.emissive + diff_comp;
  primary.a = 1.0;
  vec4 final = primary * tex_colour + spec_comp;
  final.a = 1.0;
  // *********************************
  return final;
}

// Spot light calculation
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                    in vec4 tex_colour) {
  // *********************************
  // Calculate direction to the light
  vec3 light_dir = normalize(spot.position - position);
  // Calculate distance to light
  float dist = distance(spot.position, position);
  // Calculate attenuation value
  float att_factor = 1/(spot.constant + (spot.linear * dist) + (spot.quadratic*(dist*dist)));
  // Calculate spot light intensity
  float light_intensity = pow(max(dot((-spot.direction),light_dir),0.0), spot.power);
  // Calculate light colour
   vec4 light_colour = light_intensity * att_factor * spot.light_colour;
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
  vec4 diff_comp = max(dot(normal, light_dir),0.0) * mat.diffuse_reflection * light_colour;
  // Calculate half vector
  vec3 half_vec = normalize(view_dir + light_dir);
  // Calculate specular component
  vec4 spec_comp = pow(max(dot(normal, half_vec),0.0), mat.shininess) * mat.specular_reflection * light_colour;
  // Calculate primary colour component
  vec4 primary = mat.emissive + diff_comp;
  primary.a = 1.0;
  // Calculate final colour - remember alpha
  vec4 final = primary * tex_colour + spec_comp;
  final.a = 1.0;
  // *********************************
  return final;
}
