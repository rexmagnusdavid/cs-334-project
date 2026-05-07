#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_space_matrix;

out vec3 frag_normal;
out vec3 frag_world_pos;
out vec4 frag_light_space;

void main() {
    vec4 world     = model * vec4(position, 1.0);
    frag_world_pos = world.xyz;
    frag_normal    = mat3(transpose(inverse(model))) * normal;
    frag_light_space = light_space_matrix * world;
    gl_Position    = projection * view * world;
}
