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

vec4 mask;

void main() {
	color = spriteColor * texture(image, TexCoords);
	if(isAlphaMasking == 1) {
       mask = texture(maskimage, TexCoords);
       mask.a = 1.0 - mask.r;
       mask.r = 1.0;
       mask.g = 1.0;
       mask.b = 1.0;
       color *= mask;
    }
}
