#version 330 core

#include "noise.glsl"
#include "shadow_common.glsl"

in vec3 frag_normal;
in vec3 frag_world_pos;
in vec4 frag_light_space;

uniform vec3 light_dir;

out vec4 out_color;

void main() {
    vec3  n       = normalize(frag_normal);
    vec3  l       = normalize(-light_dir);
    float geo_s   = calc_shadow(frag_light_space);
    float cloud_s = calc_cloud_shadow(frag_world_pos, l);
    float shadow  = clamp(geo_s + cloud_s * 0.40 * (1.0 - geo_s), 0.0, 1.0);
    float ambient = 0.20;
    float diffuse = max(dot(n, l), 0.0) * 0.80;

    vec3 stone = vec3(0.50, 0.47, 0.43);
    out_color  = vec4(stone * (ambient + (1.0 - shadow) * diffuse), 1.0);
}
