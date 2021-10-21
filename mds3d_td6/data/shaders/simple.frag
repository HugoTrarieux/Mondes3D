#version 330 core

in vec3 v_color;
in vec3 v_normal;
in vec3 v_view;
in vec2 v_texcoord;

uniform sampler2D earth_tex_0;
uniform sampler2D clouds_tex_1;
uniform sampler2D night_tex_2;
uniform sampler2D checkboard_tex_3;
uniform sampler2D cow_tex_4;

uniform vec3 lightDir;

out vec4 out_color;

vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s)
{
  vec3 res = vec3(0,0,0);
  float dc = max(0,dot(n,l));
  if(dc>0) {
    res = dCol * dc;
    float sc = max(0,dot(n,normalize(v+l)));
    if(sc>0)
      res += sCol * pow(sc,s) * dc;
  }
  return res;
}

void main(void) {
  float ambient = 0.0;
  float shininess = 50;
  vec3 spec_color = vec3(1,1,1);
  float light = max(dot(v_normal, lightDir), 0);

  vec4 earth = light * texture(earth_tex_0, v_texcoord);
  vec4 clouds = light * texture(clouds_tex_1, v_texcoord);
  vec4 earthClouds = mix(earth, clouds, 0.5);
  vec4 night = texture(night_tex_2, v_texcoord);
  vec4 earthNight = mix(night, earthClouds, light);

  vec4 checkboard = texture(checkboard_tex_3, v_texcoord);

  vec4 cow = texture(cow_tex_4, v_texcoord);

  //out_color = vec4(v_texcoord, 0, 0);   //Sphere jaune/verte
  //out_color = earth;                     //Terre
  //out_color = earthClouds;              //Terre+Nuages
  //out_color = earthNight;

  //out_color = checkboard;

  out_color = cow;

  //out_color = vec4(ambient * v_color + blinn(normalize(v_normal),normalize(v_view), lightDir, v_color, spec_color, shininess),1.0);
}
