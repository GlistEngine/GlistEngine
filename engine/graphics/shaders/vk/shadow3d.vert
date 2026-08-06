#version 450

// Depth-only pass of the shadow map: the scene as the light sees it. The depth
// buffer this writes is the entire output; the texture coordinate is passed on only
// so the fragment stage can punch cutout holes in it, and is not used otherwise.
//
// The vertex layout is still the full gVertex one, because these are the same
// buffers the shading pass draws from; the pipeline simply declares only the
// attributes this shader consumes.

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;
// Same per-instance binding as mesh3d.vert, so an instanced mesh casts as many
// shadows as it draws copies. Non-instanced draws bind a one-element identity.
layout(location = 6) in mat4 aInstanceModel;

layout(push_constant) uniform Push {
    // lightProjection * lightView * model, multiplied on the CPU. Keeping the three
    // separate would cost 192 bytes of a 128 byte budget; the instance matrix below
    // is the only part that cannot be folded in, since it differs per instance.
    mat4 lightmodel;
    // Declared here as well even though this stage never reads it: a push constant
    // block has to be described identically in every stage that names it.
    vec4 misc;
} pc;

layout(location = 0) out vec2 vTexCoords;

void main() {
    vTexCoords = aTexCoords;
    gl_Position = pc.lightmodel * aInstanceModel * vec4(aPos, 1.0);
}
