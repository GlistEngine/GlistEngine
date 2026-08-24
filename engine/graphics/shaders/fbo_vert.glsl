// Resolves a framebuffer to the screen: a quad already in clip space, with its
// texture coordinate passed straight through.
//
// Three flat dialect blocks rather than a nested #if: gShader's preprocessor
// handles one level, and defines exactly one of VULKAN / GLES / GLCORE.
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

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

#if VULKAN
layout (location = 0) out vec2 TexCoords;
#endif
#if GLES
out vec2 TexCoords;
#endif
#if GLCORE
out vec2 TexCoords;
#endif

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
