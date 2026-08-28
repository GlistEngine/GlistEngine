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

// See image_vert.glsl. OpenGL carries four mask modes and a free rotation;
// Vulkan carries the one mask mode its 2D path currently issues.

#if VULKAN
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec4 vColor;
layout(location = 0) out vec4 outColor;

// The image itself is set 0, the alpha mask set 1. Two sets rather than two
// bindings in one, so an image and its mask each keep the descriptor set they
// already own and nothing has to be rewritten when a mask is attached or swapped.
// With no mask the image's own set is bound in both slots and the branch below is
// simply not taken.
layout(set = 0, binding = 0) uniform sampler2D tex;
layout(set = 1, binding = 0) uniform sampler2D maskTex;

// The one thing that still cannot vary per vertex: which of the two samplers the
// branch reads. It stays a push constant, and the batcher treats it as part of
// the batch key, so a masked and an unmasked run never share a draw.
layout(push_constant) uniform Push {
    int masking;
} pc;

void main() {
    outColor = vColor * texture(tex, vUV);
    // The same rule as the OpenGL image shader: the mask's red channel, inverted,
    // scales the alpha. The mask is sampled with the image's texture coordinates,
    // so a sub-part draw masks exactly the region it draws.
    if (pc.masking == 1) {
        outColor.a *= 1.0 - texture(maskTex, vUV).r;
    }
}
#endif

#if OPENGL
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform sampler2D maskimage;

uniform vec4 spriteColor;
uniform int isAlphaMasking;
uniform int maskMode;

uniform float maskCos;
uniform float maskSin;
uniform float imageCos;
uniform float imageSin;

const int MASKMODE_BOTH = 0;
const int MASKMODE_PICTURE = 1;
const int MASKMODE_MASK = 2;
const int MASKMODE_BOTH_FREE_ROTATE = 3;

void main() {
    if(isAlphaMasking == 0) {
        color = spriteColor * texture(image, TexCoords);
        return;
    }

    vec2 pivot = vec2(0.5, 0.5);
    vec2 uv = TexCoords - pivot;

    vec2 maskRotated = vec2(
        uv.x * maskCos + uv.y * maskSin,
       -uv.x * maskSin + uv.y * maskCos
    ) + pivot;

    vec2 imageRotated = vec2(
        uv.x * imageCos + uv.y * imageSin,
       -uv.x * imageSin + uv.y * imageCos
    ) + pivot;

    vec4 tex;
    vec4 msk;

    if(maskMode == MASKMODE_BOTH) {
        tex = texture(image, TexCoords);
        msk = texture(maskimage, TexCoords);
    }
    else if(maskMode == MASKMODE_PICTURE) {
        tex = texture(image, imageRotated);
        msk = texture(maskimage, TexCoords);
    }
    else if(maskMode == MASKMODE_MASK) {
        tex = texture(image, TexCoords);
        msk = texture(maskimage, maskRotated);
    }
    else if(maskMode == MASKMODE_BOTH_FREE_ROTATE) {
        tex = texture(image, imageRotated);
        msk = texture(maskimage, maskRotated);
    }
    else {
        tex = texture(image, TexCoords);
        msk = texture(maskimage, TexCoords);
    }

    float maskAlpha = 1.0 - msk.r;
    color = spriteColor * vec4(tex.rgb, tex.a * maskAlpha);
}
#endif
