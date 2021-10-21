#version 330 core

in vec3 v_normal;
in vec2 v_uv;

uniform vec3 lightDir;
uniform sampler2D colormap;

out vec4 out_color;

vec3 shade(vec3 n, vec3 l, vec3 dCol)
{
  // cosine shading
  float dc = 0.5 * dot(n,l) + 0.5;
  return dc*dCol;
}

void main(void) {
  float ambient = 0.3;

  vec3 texColor = texture(colormap, v_uv).xyz;

  vec3 col = shade(normalize(v_normal), lightDir, texColor);

  out_color = vec4(ambient * texColor + col,1.0);
}
