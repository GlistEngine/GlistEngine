#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aColor;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 ambientProduct;
    vec4 diffuseProduct;
    vec4 materialSpecularShininess;
    vec4 cameraPositionPbr;
} pc;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vNormal;
layout(location = 3) out vec3 vWorldPos;

void main() {
    gl_Position = pc.mvp * vec4(aPos, 1.0);
    // Glist's camera matrices use OpenGL's -1..1 clip-space depth. Vulkan uses
    // 0..1, so remap z while preserving the perspective divide.
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
	// Skyboxes use the same xyww convention as the OpenGL skybox shader: their
	// depth is always the far plane, so they cannot cover scene geometry.
	if((uint(pc.cameraPositionPbr.w + 0.5) & 64u) != 0u) gl_Position.z = gl_Position.w;
    vColor = vec4(aColor, 1.0);
    vUV = aUV;
    vNormal = normalize(aNormal);
    vWorldPos = aPos;
}
