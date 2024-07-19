#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

void main() {
    FragColor = texture(texture_diffuse1, TexCoords);
}