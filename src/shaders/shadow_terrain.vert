#version 330 core

#include "noise.glsl"

layout (location = 0) in vec2 position;

uniform mat4  light_space_matrix;
uniform vec2  world_offset;
uniform float frequency;
uniform float amplitude;
uniform int   octaves;

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
    vec2  world_xz = position + world_offset;
    float h        = terrain_h(world_xz);
    gl_Position    = light_space_matrix * vec4(world_xz.x, h, world_xz.y, 1.0);
}
