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

// The skybox, in both dialects.
//
// The two backends draw it differently and the blocks below are whole shaders
// rather than variations on one: OpenGL samples a cubemap with the vertex position
// as the direction, while Vulkan draws six ordinary textured quads around the
// camera and samples one 2D face per draw. Sharing the file keeps them in one
// place; it does not make them the same shader.

#if VULKAN
// The skybox, drawn as six textured quads around the camera rather than through a
// cubemap. Vulkan has cube image views, but nothing else in this backend needs one
// yet, and six ordinary 2D faces reuse the texture path that already exists.
//
// Vertices come from the per-frame dynamic ring, like the 2D path, because the
// quads are rebuilt every frame around wherever the camera now is.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(push_constant) uniform Push {
    // projection * view, already multiplied out. The quads are built in world space
    // around the camera, so there is no model matrix.
    mat4 viewprojection;
} pc;

layout(location = 0) out vec2 vTexCoords;

void main() {
    vTexCoords = aTexCoords;
    gl_Position = pc.viewprojection * vec4(aPos, 1.0);
}
#endif

#if OPENGL
layout (location = 0) in vec3 aPos;
uniform int aIsHDR;
out vec3 TexCoords;
flat out int mIsHDR;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main() {
    mIsHDR = aIsHDR;
    TexCoords = aPos;
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
#endif
