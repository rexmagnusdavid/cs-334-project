#version 330 core

in vec2 frag_uv;

uniform sampler2D occlusion_tex;
uniform vec2  sun_pos;
uniform float exposure;
uniform float decay;
uniform float density;

out vec4 out_color;

void main() {
    const int SAMPLES = 80;
    vec2  delta     = (frag_uv - sun_pos) / float(SAMPLES) * density;
    vec2  coord     = frag_uv;
    float illum     = 0.0;
    float decay_acc = 1.0;

    for (int i = 0; i < SAMPLES; i++) {
        coord  -= delta;
        illum  += texture(occlusion_tex, coord).r * decay_acc;
        decay_acc *= decay;
    }

    out_color = vec4(vec3(1.0, 0.97, 0.92) * illum * exposure, 1.0);
}
