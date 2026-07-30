#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif
layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 instanceModel;

uniform mat4 lightMatrix;
uniform mat4 model;
uniform int useInstancing;

void main() {
    gl_Position = lightMatrix * (useInstancing == 1 ? model * instanceModel : model) * vec4(aPos, 1.0);
}
