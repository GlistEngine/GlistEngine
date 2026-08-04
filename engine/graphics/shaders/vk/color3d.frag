#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vWorldPos;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseMap;

layout(push_constant) uniform Push {
    mat4 mvp;
    vec4 ambientProduct;
    vec4 diffuseProduct;
    vec4 lightDirectionTextured;
    vec4 cameraPositionPbr;
} pc;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness * roughness * roughness;
    float nh2 = max(dot(N, H), 0.0); nh2 *= nh2;
    float d = nh2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float geometrySchlickGGX(float nv, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    return nv / (nv * (1.0 - k) + k);
}

vec3 fresnelSchlick(float cosine, vec3 f0) {
    return f0 + (1.0 - f0) * pow(max(1.0 - cosine, 0.0), 5.0);
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(pc.lightDirectionTextured.xyz);
    if(pc.cameraPositionPbr.w > 0.5) {
        vec3 albedo = pc.lightDirectionTextured.w > 0.5
                ? pow(texture(diffuseMap, vUV).rgb, vec3(2.2)) : vec3(0.5);
        const float metallic = 0.0;
        const float roughness = 0.5;
        vec3 V = normalize(pc.cameraPositionPbr.xyz - vWorldPos);
        vec3 H = normalize(V + L);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), vec3(0.04));
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySchlickGGX(max(dot(N, V), 0.0), roughness)
                * geometrySchlickGGX(max(dot(N, L), 0.0), roughness);
        vec3 specular = NDF * G * F
                / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
        float ndl = max(dot(N, L), 0.0);
        vec3 radiance = pc.diffuseProduct.rgb * PI;
        vec3 color = pc.ambientProduct.rgb * albedo
                + (kD * albedo / PI + specular) * radiance * ndl;
        color = color / (color + vec3(1.0));
        outColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
    } else {
        float diffuse = max(dot(N, L), 0.0);
        outColor = vColor * (pc.ambientProduct + pc.diffuseProduct * diffuse);
        if(pc.lightDirectionTextured.w > 0.5) outColor *= texture(diffuseMap, vUV);
    }
}
