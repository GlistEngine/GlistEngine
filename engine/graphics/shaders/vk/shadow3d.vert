#version 450

layout(location = 0) in vec3 aPos;

layout(push_constant) uniform ShadowPush {
    mat4 lightMatrix;
} pc;

void main() {
    gl_Position = pc.lightMatrix * vec4(aPos, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
}
