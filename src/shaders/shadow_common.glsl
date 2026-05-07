uniform sampler2D shadow_map;
uniform float     cloud_altitude;

float calc_shadow(vec4 light_space) {
    vec3 proj = light_space.xyz / light_space.w * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;
    float bias   = 0.003;
    float shadow = 0.0;
    vec2  tx     = 1.0 / textureSize(shadow_map, 0);
    for (int x = -2; x <= 2; x++)
        for (int y = -2; y <= 2; y++)
            shadow += (proj.z - bias) > texture(shadow_map, proj.xy + vec2(x, y) * tx).r ? 1.0 : 0.0;
    return shadow / 25.0;
}

float calc_cloud_shadow(vec3 world_pos, vec3 sun_dir) {
    if (sun_dir.y <= 0.0 || world_pos.y >= cloud_altitude) return 0.0;
    float t   = (cloud_altitude - world_pos.y) / sun_dir.y;
    vec2  cxz = world_pos.xz + t * sun_dir.xz;
    return smoothstep(-0.05, 0.25, fbm(cxz, 4, 0.0018));
}
