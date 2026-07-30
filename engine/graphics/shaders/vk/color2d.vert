#version 450

// Minimal 2D coloured-geometry shader for the Vulkan backend (gDrawTriangle /
// gDrawRectangle). Deliberately separate from the OpenGL color_vert.glsl: that
// one carries the full 3D lighting/fog/shadow machinery and uses default-block
// uniforms, which Vulkan/SPIR-V forbids. Here everything rides in one push
// constant, so no vertex attributes beyond position and no descriptor sets.

layout(location = 0) in vec2 aPos;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 color;
} pc;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = pc.mvp * vec4(aPos, 0.0, 1.0);
    vColor = pc.color;
}
