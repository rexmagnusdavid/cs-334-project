#version 330 core

#include "noise.glsl"

in vec2 frag_world_xz;

out vec4 out_color;

void main() {
    float shape      = fbm(frag_world_xz, 4, 0.0018);
    float alpha      = smoothstep(-0.05, 0.25, shape) * 0.92;
    if (alpha < 0.01) discard;

    float detail     = fbm(frag_world_xz, 4, 0.006) * 0.5 + 0.5;
    float brightness = 0.86 + detail * 0.14;
    out_color = vec4(vec3(brightness), alpha);
}
