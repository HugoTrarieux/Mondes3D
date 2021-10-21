#version 330 core

in vec3 vtx_position;
in vec3 vtx_color;
in vec3 vtx_normal;

out vec3 var_color;

uniform mat4 obj_mat;
uniform mat4 view_mat;
uniform mat4 pspt_mat;
uniform mat3 normal_mat;
uniform int wireframe;
uniform vec3 light_pos;

out vec3 light_dir;
out vec3 color;
out vec3 normal;
out vec3 view_pos;

void main()
{
  gl_Position = pspt_mat * view_mat * obj_mat * vec4(vtx_position.xyz, 1.);
  
  vec3 n = normalize(normal_mat * vtx_normal);
  vec4 v = view_mat * obj_mat * vec4(vtx_position, 1);
  vec4 l4 = view_mat * vec4(light_pos - v.xyz, 1);
  vec3 l = normalize(l4.xyz);

  light_dir = l;
  view_pos = v.xyz;
  normal = n;
  color = vtx_color;

  if(wireframe == 1)
    var_color = vec3(1,1,1);
  else
    var_color = vtx_color;
}
