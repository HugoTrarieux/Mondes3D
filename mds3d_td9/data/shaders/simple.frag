#version 330 core

in vec3 v_normal;
in vec3 v_view;
in vec3 v_col;

uniform vec3 lightDir;
uniform int wireframe;

out vec4 out_color;

vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s)
{
  vec3 res = vec3(0,0,0);
  float dc = max(0,dot(n,l));
  if(dc>0) {
    res = dCol * dc;
  }
  return res;
}

void main(void) {
  float ambient = 0.4;
  float shininess = 50;
  vec3 spec_color = vec3(1,1,1);

  vec3 blinnColor = blinn(normalize(v_normal),normalize(v_view), lightDir, v_col, spec_color, shininess);

  if(wireframe==1)
    out_color = vec4(0.9,0.1,0.1,1);
  else
    out_color = vec4(ambient * v_col + blinnColor,1.0);
}
