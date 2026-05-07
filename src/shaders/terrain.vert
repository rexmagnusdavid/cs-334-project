#version 330 core

#include "noise.glsl"

layout (location = 0) in vec2 position;

uniform mat4  projection;
uniform mat4  view;
uniform mat4  light_space_matrix;
uniform vec2  world_offset;
uniform float frequency;
uniform float amplitude;
uniform int   octaves;

out vec3  frag_world_pos;
out vec3  frag_normal;
out float frag_height;
out float frag_continent;
out vec4  frag_light_space;

float continent_mask(vec2 p) {
    float c    = fbm(p, 4, 0.003);
    float base = smoothstep(-0.05, 0.42, c);
    return base * base;
}

float terrain_h(vec2 p) {
    float continent = continent_mask(p);
    float detail    = fbm(p, octaves, frequency);
    return continent * (1.0 + detail * 0.4) * amplitude;
}

void main() {
    vec2 world_xz = position + world_offset;

    const float e = 0.1;
    float h   = terrain_h(world_xz);
    float h_x = terrain_h(world_xz + vec2(e, 0.0));
    float h_z = terrain_h(world_xz + vec2(0.0, e));

    frag_world_pos   = vec3(world_xz.x, h, world_xz.y);
    frag_height      = h;
    frag_continent   = continent_mask(world_xz);
    frag_normal      = normalize(vec3(h - h_x, e, h - h_z));
    frag_light_space = light_space_matrix * vec4(frag_world_pos, 1.0);

    gl_Position = projection * view * vec4(frag_world_pos, 1.0);
}
