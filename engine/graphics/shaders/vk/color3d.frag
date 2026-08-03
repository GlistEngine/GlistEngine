#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 color;
    int textured;
} pc;

void main() {
    outColor = vColor;
    if(pc.textured == 1) outColor *= texture(diffuseMap, vUV);
    // A small object-space key light gives the first material path readable
    // volume and, importantly, keeps normals in the reflected vertex interface.
    // A world-space normal matrix will replace this when the light UBO lands.
    float diffuse = max(dot(normalize(vNormal), normalize(vec3(0.4, 0.8, 0.5))), 0.0);
    outColor.rgb *= 0.35 + 0.65 * diffuse;
}
