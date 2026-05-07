#version 330 core

layout (location = 0) in vec2 position;

uniform mat4  projection;
uniform mat4  view;
uniform vec3  center;
uniform float size;

out vec2 frag_world_xz;

void main() {
    vec3 world    = vec3(center.x + position.x * size, center.y, center.z + position.y * size);
    frag_world_xz = world.xz;
    gl_Position   = projection * view * vec4(world, 1.0);
}
