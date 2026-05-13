#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

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