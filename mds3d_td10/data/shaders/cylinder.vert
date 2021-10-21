#version 330 core

const float M_PI = 3.14159265359;

uniform mat4 obj_mat;
uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat3 normal_mat;

in vec3 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;

out vec3 v_normal;
out vec3 v_view;
out vec2 v_uv;

vec3 cylinder(vec2 uv, float r, float l)
{
    return vec3(r*cos(uv.x*2*M_PI), r*sin(uv.x*2*M_PI), l*uv.y);
}

void main()
{
    v_uv  = vtx_texcoord;
    vec3 pos = cylinder(v_uv, 1, 5);
    vec3 normal = vec3(pos.x,pos.y,0);

    v_normal = normalize(normal_mat * normal);
    vec4 p = view_mat * (obj_mat * vec4(pos, 1.));
    v_view = normalize(-p.xyz);
    gl_Position = proj_mat * p;
}
