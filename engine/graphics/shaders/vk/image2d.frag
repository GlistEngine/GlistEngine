#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 tint;
} pc;

void main() {
    outColor = pc.tint * texture(tex, vUV);
}
