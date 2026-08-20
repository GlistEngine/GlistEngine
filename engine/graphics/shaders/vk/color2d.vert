#version 450

// Minimal 2D coloured-geometry shader for the Vulkan backend (gDrawTriangle /
// gDrawRectangle). Deliberately separate from the OpenGL color_vert.glsl: that
// one carries the full 3D lighting/fog/shadow machinery and uses default-block
// uniforms, which Vulkan/SPIR-V forbids.
//
// The transform and the colour used to be push constants, which made every 2D
// shape its own draw call: a push constant cannot vary within a draw. They are
// baked into the vertices instead, so a run of shapes becomes one draw. The
// position arrives already multiplied through - see gvk2DVertex in gVKDraw.cpp -
// which is why there is no matrix here at all.

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = aPos;
    vColor = aColor;
}
