#version 450

// Minimal 2D textured-quad shader for the Vulkan backend (gImage / gTexture).
// Separate from the OpenGL image_vert.glsl for the same reason as color2d: the
// GL version relies on default-block uniforms that Vulkan/SPIR-V rejects. The
// transform and tint colour ride in a push constant; the texture is a single
// combined image sampler in descriptor set 0.

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 tint;
} pc;

layout(location = 0) out vec2 vUV;

void main() {
    gl_Position = pc.mvp * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
