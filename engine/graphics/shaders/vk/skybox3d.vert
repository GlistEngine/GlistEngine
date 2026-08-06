#version 450

// The skybox, drawn as six textured quads around the camera rather than through a
// cubemap. Vulkan has cube image views, but nothing else in this backend needs one
// yet, and six ordinary 2D faces reuse the texture path that already exists - the
// same trade Mehmet's branch makes.
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
