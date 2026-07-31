#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    float outlineoffset = 0.03;
    vec3 currentpos = aPos + aNormal * outlineoffset;

    gl_Position = projection * view * model * vec4(currentpos, 1.0);
}