#version 330 core

layout (location = 0) in vec3 position;

uniform mat4  projection;
uniform mat4  view;
uniform vec3  center;
uniform float radius;

void main() {
    gl_Position = projection * view * vec4(position * radius + center, 1.0);
}
