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

// The 2D textured quad (gImage, gTexture), in both dialects.
//
// The blocks are whole shaders rather than one shader declared twice. OpenGL puts
// the transform and the tint in uniforms, which costs a draw call per image;
// Vulkan bakes both into the vertex so a run of images sharing a texture collapses
// into one draw. Making OpenGL do the same is a change to its draw path, not to
// this file.

#if VULKAN
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
#endif

#if OPENGL
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>\n"

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform bool isSubPart;
uniform vec2 subPos;
uniform vec2 subScale;

void main() {
    if (isSubPart) {
        TexCoords = (vertex.zw + subPos) / subScale;
    } else {
        TexCoords = vertex.zw;
    }
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
#endif
