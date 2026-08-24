#if VULKAN
#version 450
#endif
#if GLES
#version 300 es
precision highp float;
#endif
#if GLCORE
#version 330 core
#endif

// Vulkan has no default uniform block: a sampler needs a set and a binding, and
// every stage input and output needs a location. The declarations differ; the
// code below does not.
#if VULKAN
layout (location = 0) in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;
layout (set = 0, binding = 0) uniform sampler2D screenTexture;
#endif
#if GLES
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
#endif
#if GLCORE
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
#endif

void main() {
    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
}
