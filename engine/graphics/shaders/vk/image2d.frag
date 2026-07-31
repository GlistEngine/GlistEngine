#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

// The image itself is set 0, the alpha mask set 1. Two sets rather than two
// bindings in one, so an image and its mask each keep the descriptor set they
// already own and nothing has to be rewritten when a mask is attached or swapped.
// With no mask the image's own set is bound in both slots and the branch below is
// simply not taken.
layout(set = 0, binding = 0) uniform sampler2D tex;
layout(set = 1, binding = 0) uniform sampler2D maskTex;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 tint;
    int masking;
} pc;

void main() {
    outColor = pc.tint * texture(tex, vUV);
    // The same rule as the OpenGL image shader: the mask's red channel, inverted,
    // scales the alpha. The mask is sampled with the image's texture coordinates,
    // so a sub-part draw masks exactly the region it draws.
    if (pc.masking == 1) {
        outColor.a *= 1.0 - texture(maskTex, vUV).r;
    }
}
