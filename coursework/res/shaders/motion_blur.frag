#version 440 core

// Current frame being rendered
uniform sampler2D tex;
// Previous frame
uniform sampler2D previous_frame;

// Blend factor between frames
uniform float blend_factor;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample the two textures
  vec4 tex_colour = texture(tex, tex_coord);
  vec4 frame_colour = texture(previous_frame, tex_coord);
  // Mix between these two colours
  colour = mix(frame_colour, tex_colour, 0.3);
  // Ensure alpha is 1.0
  colour.a = 1.0;
  // *********************************
}
