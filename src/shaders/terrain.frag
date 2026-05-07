#version 330 core

#include "noise.glsl"
#include "shadow_common.glsl"

in vec3  frag_world_pos;
in vec3  frag_normal;
in float frag_height;
in float frag_continent;
in vec4  frag_light_space;

uniform float amplitude;
uniform float time;
uniform vec3  light_dir;
uniform vec3  cam_pos;

out vec4 out_color;

vec3 bump_normal(vec3 n, vec2 xz, float freq, float strength) {
    const float e = 0.8;
    float h0 = fbm(xz,               3, freq);
    float hx = fbm(xz + vec2(e, 0.0), 3, freq);
    float hz = fbm(xz + vec2(0.0, e), 3, freq);
    return normalize(n + vec3((h0-hx)/e, 0.0, (h0-hz)/e) * strength);
}

vec3 sand_color(vec2 xz) {
    float n = fbm(xz, 3, 0.80) * 0.5 + 0.5;
    return mix(vec3(0.72, 0.64, 0.42), vec3(0.86, 0.78, 0.55), n);
}

vec3 grass_color(vec2 xz) {
    float n  = fbm(xz,                   4, 0.22) * 0.5 + 0.5;
    float n2 = fbm(xz + vec2(5.3, 9.1),  3, 0.08) * 0.5 + 0.5;
    vec3 base = mix(vec3(0.14, 0.38, 0.09), vec3(0.28, 0.58, 0.18), n);
    return mix(base, vec3(0.42, 0.52, 0.12), n2 * 0.25);
}

vec3 rock_color(vec2 xz) {
    float n  = fbm(xz,                    4, 0.14) * 0.5 + 0.5;
    float n2 = fbm(xz + vec2(17.2, 31.4), 3, 0.06) * 0.5 + 0.5;
    vec3 base = mix(vec3(0.36, 0.32, 0.28), vec3(0.54, 0.49, 0.43), n);
    return mix(base, vec3(0.44, 0.38, 0.31), n2 * 0.35);
}

vec3 snow_color(vec2 xz) {
    float n = fbm(xz, 3, 0.35) * 0.5 + 0.5;
    return mix(vec3(0.87, 0.91, 0.97), vec3(0.96, 0.97, 1.00), n);
}

vec3 water_wave_normal(vec2 xz) {
    vec2 f1 = xz * 0.024 + vec2(time * 0.08, time * 0.05);
    vec2 f2 = xz * 0.041 - vec2(time * 0.06, time * 0.10);
    const float e = 1.5;
    float h0 = fbm(f1,           4, 1.0) + fbm(f2,           3, 1.0) * 0.5;
    float hx = fbm(f1 + vec2(e,0), 4, 1.0) + fbm(f2 + vec2(e,0), 3, 1.0) * 0.5;
    float hz = fbm(f1 + vec2(0,e), 4, 1.0) + fbm(f2 + vec2(0,e), 3, 1.0) * 0.5;
    return normalize(vec3((h0-hx)/e * 1.8, 1.0, (h0-hz)/e * 1.8));
}

void main() {
    vec3  l       = normalize(-light_dir);
    float geo_s   = calc_shadow(frag_light_space);
    float cloud_s = calc_cloud_shadow(frag_world_pos, l);
    float shadow  = clamp(geo_s + cloud_s * 0.40 * (1.0 - geo_s), 0.0, 1.0);
    float lit     = 1.0 - shadow;
    vec2  xz      = frag_world_pos.xz;

    if (frag_continent < 0.02) {
        vec3  view_dir = normalize(cam_pos - frag_world_pos);
        vec3  wn       = water_wave_normal(xz);
        float fresnel  = pow(1.0 - max(dot(view_dir, wn), 0.0), 4.0);
        vec3  half_v   = normalize(l + view_dir);
        float spec     = pow(max(dot(wn, half_v), 0.0), 90.0) * lit * 0.90;
        float diffuse  = max(dot(wn, l), 0.0) * 0.60;
        vec3  deep     = vec3(0.03, 0.14, 0.42);
        vec3  shallow  = vec3(0.06, 0.30, 0.62);
        vec3  sky_r    = vec3(0.42, 0.72, 0.88);
        vec3  wcolor   = mix(deep, shallow, wn.y * 0.5 + 0.3);
        wcolor         = mix(wcolor, sky_r, fresnel * 0.45);
        out_color = vec4(wcolor * (0.28 + lit * diffuse) + vec3(1.0, 0.97, 0.92) * spec, 1.0);
        return;
    }

    float t       = clamp(frag_height / amplitude, 0.0, 1.0);
    float ambient = 0.20;
    vec3  n       = normalize(frag_normal);
    vec3  base_color;
    float spec    = 0.0;

    if (t < 0.10) {
        float blend = smoothstep(0.00, 0.10, t);
        base_color  = mix(sand_color(xz), grass_color(xz), blend);

    } else if (t < 0.45) {
        float blend = smoothstep(0.10, 0.45, t);
        base_color  = mix(grass_color(xz), rock_color(xz), blend);
        vec3 gbump  = bump_normal(n, xz, 0.28, mix(0.0, 0.4, blend));
        vec3 rbump  = bump_normal(n, xz, 0.16, mix(0.4, 1.8, blend));
        n           = mix(gbump, rbump, blend);

    } else {
        float blend    = smoothstep(0.45, 0.80, t);
        base_color     = mix(rock_color(xz), snow_color(xz), blend);
        n              = bump_normal(n, xz, 0.16, 1.8 * (1.0 - blend));
        vec3 view_dir  = normalize(cam_pos - frag_world_pos);
        vec3 half_v    = normalize(l + view_dir);
        spec = pow(max(dot(n, half_v), 0.0), 28.0) * smoothstep(0.55, 0.80, t) * 0.22;
    }

    float diffuse = max(dot(n, l), 0.0) * 0.80;
    out_color = vec4(base_color * (ambient + lit * diffuse) + spec * lit, 1.0);
}
