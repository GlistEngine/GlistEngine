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

// Depth-only pass of the shadow map: the scene as the light sees it. The depth
// buffer this writes is the entire output.
//
// The vertex layout is the full gVertex one, because these are the same buffers
// the shading pass draws from; the pipeline declares only the attributes this
// shader consumes.

layout (location = 0) in vec3 aPos;
// Same per-instance binding as the shading pass, so an instanced mesh casts as
// many shadows as it draws copies.
layout (location = 6) in mat4 instanceModel;

#if VULKAN
layout(push_constant) uniform Push {
    // lightProjection * lightView * model, multiplied on the CPU. Keeping the
    // three separate would cost 192 bytes of a 128 byte budget; the instance
    // matrix is the only part that cannot be folded in, since it differs per
    // instance. A non-instanced draw binds a one-element identity.
    mat4 lightmodel;
} pc;
#endif
#if OPENGL
uniform mat4 lightMatrix;
uniform mat4 model;
uniform int useInstancing;
#endif

void main() {
#if VULKAN
    gl_Position = pc.lightmodel * instanceModel * vec4(aPos, 1.0);
#endif
#if OPENGL
    gl_Position = lightMatrix * (useInstancing == 1 ? model * instanceModel : model) * vec4(aPos, 1.0);
#endif
}
