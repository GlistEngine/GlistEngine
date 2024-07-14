#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (location = 0) in vec3 aPos;
uniform int aIsHDR;
out vec3 TexCoords;
flat out int mIsHDR;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main() {
    mIsHDR = aIsHDR;
    TexCoords = aPos;
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}