#version 450

// Resolves a framebuffer to the screen: a quad already in clip space, with its
// texture coordinate passed straight through. This is the Vulkan branch of
// graphics/shaders/fbo_vert.glsl, kept here so the SPIR-V can be built ahead of
// time - a release build links no shader compiler, and gFbo is the one shader
// gRenderer::init() creates that the Vulkan backend needs as well. The two must
// stay in step: an edit to one belongs in the other.

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (location = 0) out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
