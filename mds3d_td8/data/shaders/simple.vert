#version 330 core

#define M_PI 3.14159265358979323846

uniform mat4 obj_mat;
uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat3 normal_mat;

in vec3 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;

out vec3 v_normal;
out vec2 v_uv;

vec3 cylinder(vec2 uv, vec3 A, vec3 B, float r){
  vec3 v_t = B - A;
  vec3 v_tmp = vec3(1,1,1);
  vec3 v_n = cross(v_t,v_tmp);
  normalize(v_n);
  vec3 v_b = cross(v_t,v_n);
  normalize(v_b);

  float angle = 2 * M_PI * uv.x;
  float x = r * cos(angle);
  float y = r * sin(angle);

  return y * v_n + x * v_b + A + uv.y * v_t;

  //return r * vec3(v_t.x * uv.x, v_n.y * cos(2*M_PI*uv.y), v_b.x * sin(2*M_PI*uv.y));
}

vec3 bezier(float u, vec3 B[4]){
  vec3 res[4];

  res[0] = B[0];
  res[1] = B[1] * u;
  res[2] = B[2] * u * u;
  res[3] = B[3] * u * u * u;

  return res[0] + res[1] + res[2] + res[3];
}

vec3 cylBezierYZ(vec2 uv, vec3 B[4], float r){
  vec3 center = bezier(uv.x, B);
  
  float angle = 2 * M_PI * uv.y;
  float x = r * cos(angle);
  float y = r * sin(angle);
  return vec3(x, y, 0) + center;
}

void main()
{
  v_uv  = vtx_texcoord;
  v_normal = normalize(normal_mat * vtx_normal);

  vec3 B[4];
  B[0] = vec3(-1,0,2);
  B[1] = vec3(-0.3,0,4);
  B[2] = vec3(0.3,0,1);
  B[3] = vec3(1,0,-0.5);

  vec3 position = cylBezierYZ(vtx_texcoord, B, 0.1);
  //vec3 position = cylinder(v_uv, vec3(0,1,5), vec3(2,2,0), 0.2);

  vec4 p = view_mat * (obj_mat * vec4(position, 1.));
  gl_Position = proj_mat * p;
}
