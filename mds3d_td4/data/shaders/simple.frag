#version 330 core

in vec3 var_color;
in vec3 light_dir;
in vec3 view_pos;
in vec3 color;
in vec3 normal;

out vec4 out_color;

uniform int wireframe;


vec3 blinn(vec3 n, vec3 v, vec3 l, vec3 dCol, vec3 sCol, float s) {
    float diffuse = max(dot(n,l),0);
    vec3 h = normalize(l + v);
    float specular = pow((max(dot(n,h),0)),s);
    vec3 pvl = dCol + sCol * specular;
    vec3 phong = vec3(0,0,0);

    // --- Specular --- //
    //vec3 pvl = sCol * specular;
    //phong = phong + pvl;

    // --- Diffuse --- //
    //vec3 pvl = dCol;

    phong = phong + diffuse * pvl;
    return phong;
}

void main(void) {
    vec3 var_color = blinn(normal, normalize(-view_pos), light_dir, color, vec3(20, 10, 1), 30);
    out_color = vec4(var_color.xyz, 1);
}
