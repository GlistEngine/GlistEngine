#version 450

// The fragment half of fbo.vert: samples the resolved framebuffer and writes it
// opaque. The Vulkan branch of graphics/shaders/fbo_frag.glsl; see the note there.

layout (location = 0) in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;
layout (set = 0, binding = 0) uniform sampler2D screenTexture;

void main() {
    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
}
