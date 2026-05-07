#pragma once

#include <glm/glm.hpp>
#include <cmath>

inline glm::vec2 hash2(glm::vec2 p) {
    glm::vec3 q = glm::fract(glm::vec3(p.x, p.y, p.x) * glm::vec3(0.1031f, 0.1030f, 0.0973f));
    q += glm::dot(q, glm::vec3(q.y, q.z, q.x) + 33.33f);
    return -1.0f + 2.0f * glm::fract(glm::vec2(q.x + q.y, q.x + q.z) * glm::vec2(q.z, q.y));
}

inline float perlin(glm::vec2 p) {
    glm::vec2 i = glm::floor(p);
    glm::vec2 f = glm::fract(p);
    glm::vec2 u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);

    float a = glm::dot(hash2(i + glm::vec2(0.0f, 0.0f)), f - glm::vec2(0.0f, 0.0f));
    float b = glm::dot(hash2(i + glm::vec2(1.0f, 0.0f)), f - glm::vec2(1.0f, 0.0f));
    float c = glm::dot(hash2(i + glm::vec2(0.0f, 1.0f)), f - glm::vec2(0.0f, 1.0f));
    float d = glm::dot(hash2(i + glm::vec2(1.0f, 1.0f)), f - glm::vec2(1.0f, 1.0f));

    return glm::mix(glm::mix(a, b, u.x), glm::mix(c, d, u.x), u.y);
}

inline float fbm(glm::vec2 p, int oct, float freq) {
    float value = 0.0f;
    float amp   = 0.5f;
    float total = 0.0f;
    for (int i = 0; i < oct; ++i) {
        value += perlin(p * freq) * amp;
        total += amp;
        freq  *= 2.0f;
        amp   *= 0.5f;
    }
    return value / total;
}

inline float continent_mask(glm::vec2 p) {
    float c    = fbm(p, 4, 0.003f);
    float base = glm::smoothstep(-0.05f, 0.42f, c);
    return base * base;
}

inline float terrain_h(glm::vec2 p, int octaves, float frequency, float amplitude) {
    float continent = continent_mask(p);
    float detail    = fbm(p, octaves, frequency);
    return continent * (1.0f + detail * 0.4f) * amplitude;
}
