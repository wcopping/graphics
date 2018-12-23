#version 450 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vec3 frag_pos;
out vec3 normal;
out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  frag_pos = vec3(model * vec4(a_pos, 1.0));
  normal = mat3(transpose(inverse(model))) * a_normal;  
  tex_coords = a_tex_coords;
  
  gl_Position = projection * view * vec4(frag_pos, 1.0);
}
