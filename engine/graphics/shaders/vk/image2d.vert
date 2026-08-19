#version 450

// Minimal 2D textured-quad shader for the Vulkan backend (gImage / gTexture).
// Separate from the OpenGL image_vert.glsl for the same reason as color2d: the
// GL version relies on default-block uniforms that Vulkan/SPIR-V rejects.
//
// Position and tint ride in the vertex, not in a push constant, so consecutive
// images sharing a texture collapse into a single draw. The position is already
// in clip space; the CPU applies the transform while it packs the quad.
//
// The vertex layout is the one color2d uses, declared in gVKPipeline.cpp. Both
// pipelines read the same buffer, so they have to agree on its stride.

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec2 vUV;
layout(location = 1) out vec4 vColor;

void main() {
    gl_Position = aPos;
    vUV = aUV;
    vColor = aColor;
}
