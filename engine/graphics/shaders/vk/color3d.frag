#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 ambientProduct;
    vec4 diffuseProduct;
    vec4 lightDirectionTextured;
} pc;

void main() {
    float diffuse = max(dot(normalize(vNormal), normalize(pc.lightDirectionTextured.xyz)), 0.0);
    outColor = vColor * (pc.ambientProduct + pc.diffuseProduct * diffuse);
    if(pc.lightDirectionTextured.w > 0.5) outColor *= texture(diffuseMap, vUV);
}
