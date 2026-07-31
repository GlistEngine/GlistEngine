#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float utime;

void main() {
    float glowoffset = 0.08 + sin(utime * 3.0) * 0.02;
    vec3 currentpos = aPos + aNormal * glowoffset;

    gl_Position = projection * view * model * vec4(currentpos, 1.0);
}