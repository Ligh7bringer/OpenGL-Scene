#version 430 core

// Incoming texture containing frame information
uniform sampler2D tex;

// Our colour filter - calculates colour intensity
const vec3 intensity = vec3(0.314, 0.169, -0.090);

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample texture colour
  vec4 tex_colour = texture(tex, tex_coord);
  // Calculate grey value
  vec4 greyscale = tex_colour * vec4(intensity, 1.0);
  // Use greyscale to as final colour
  // - ensure alpha is 1
  colour = greyscale;
  colour.a = 1.0;
// ********************
}
