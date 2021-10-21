#version 330 core

in vec3 v_color;
in vec3 v_normal;
in vec2 v_texcoords;
in vec3 v_view;
in vec3 v_tangent;
in vec3 v_bitangent;

uniform vec3 lightDir;

uniform sampler2D texDay;
uniform sampler2D texClouds;
uniform sampler2D texNight;
uniform sampler2D texNormals;
uniform sampler2D texDiffuse;
uniform sampler2D texEarthNormals;

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
  float ambient = 0.1;
  vec3 day_color = texture(texDay, v_texcoords).rgb;
  vec3 night_color = texture(texNight, v_texcoords).rgb;


//---- Flat ----//
  //vec4 normals_text = texture(texNormals, v_texcoords);
  //vec3 n = (normals_text.xyz*2)-1;
  //float light_normals = max(0, dot((normals_text.xyz+1)/2, lightDir));
  //vec3 diffuse_text = (texture(texDiffuse, v_texcoords)).xyz;
  //out_color = light_normals * normals_text;
  //out_color = vec4(blinn(normalize(n), normalize(v_view), lightDir, diffuse_text, vec3(1,1,1), 10), 1.0);
  //out_color = vec4(day_color, 1);

//---- Sphere ----//
  vec4 earthNormals = texture(texEarthNormals, v_texcoords);
  vec3 n = (earthNormals.xyz*2)-1;

  vec3 v_n = normalize(v_normal);
  vec3 v_t = normalize(v_tangent);
  vec3 v_b = normalize(v_bitangent);

  mat3 tbnvMatrix = transpose(mat3(v_t, v_b, v_n));

  out_color = vec4(blinn(normalize(n), tbnvMatrix * normalize(v_view), tbnvMatrix * lightDir, day_color, vec3(1,1,1), 20), 1.0);


}
