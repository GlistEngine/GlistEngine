#version 450

// Cook-Torrance PBR for the Vulkan backend, ported from pbr_frag.glsl. The OpenGL
// version has no image-based lighting - no irradiance, prefilter or BRDF LUT - so
// this is a straight port of the analytic part rather than a reduced one.
//
// Unlike the mesh3d shaders, the five maps share one descriptor set instead of
// taking one set each. Vulkan only guarantees four bound sets, and scene plus five
// textures would need six; one set with five bindings needs two.

const float PI = 3.14159265359;

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec3 vWorldPos;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec4 vFragPosLightSpace;

layout(location = 0) out vec4 outColor;

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

layout(set = 0, binding = 0) uniform Scene {
    mat4 projection;
    mat4 view;
    mat4 lightmatrix;
    vec4 viewpos;
    vec4 globalambientcolor;
    vec4 rendercolor;
    // xyz the shadow-casting light's position, w whether a shadow map is bound.
    vec4 shadowlightpos;
    int lightnum;
    int enabledlights;
    int softshadows;
    Light lights[8];
} scene;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D metallicMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;
// The shadow map gets a set of its own rather than a sixth binding in the material
// set: it is per frame, not per material, and the material sets are cached by their
// five texture ids. Three sets in total still fits the four Vulkan guarantees. Like
// the maps above this binding is always filled - with the 1x1 white texture when no
// shadow map exists - and scene.shadowlightpos.w says whether to believe it.
layout(set = 2, binding = 0) uniform sampler2D shadowmap;

layout(push_constant) uniform Push {
    mat4 model;
    // maps0 = albedo, normal, metallic, roughness; maps1.x = ao. Non-zero means the
    // map is supplied; where it is not, pbr_frag.glsl's defaults are used.
    ivec4 maps0;
    ivec4 maps1;
} pc;

// Tangent basis from screen-space derivatives rather than a vertex tangent, which
// is what lets PBR work on meshes that carry no tangent attribute at all.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, vTexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(vWorldPos);
    vec3 Q2 = dFdy(vWorldPos);
    vec2 st1 = dFdx(vTexCoords);
    vec2 st2 = dFdy(vTexCoords);

    vec3 N = normalize(vNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// Identical to mesh3d.frag's calculateShadow, down to the bias and the PCF kernel,
// so the same surface lands in shadow at the same angle whether it is shaded as PBR
// or not. See that file for why the light-space depth is not rescaled here the way
// the OpenGL shaders rescale it.
float calculateShadow(vec3 normal) {
    vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(scene.shadowlightpos.xyz - vWorldPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowmap, 0));
    float shadow = 0.0;
    int radius = scene.softshadows != 0 ? 2 : 1;
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    float taps = float((2 * radius + 1) * (2 * radius + 1));
    return shadow / taps;
}

void main() {
    // Defaults match pbr_frag.glsl exactly: mid grey albedo, dielectric, half rough,
    // fully unoccluded. The albedo map is raised to 2.2 because it is authored in
    // sRGB while the maths below is linear.
    vec3 albedo = pc.maps0.x > 0 ? pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2)) : vec3(0.5);
    float metallic = pc.maps0.z > 0 ? texture(metallicMap, vTexCoords).r : 0.0;
    float roughness = pc.maps0.w > 0 ? texture(roughnessMap, vTexCoords).r : 0.5;
    float ao = pc.maps1.x > 0 ? texture(aoMap, vTexCoords).r : 1.0;

    vec3 N = pc.maps0.y > 0 ? getNormalFromMap() : normalize(vNormal);
    vec3 V = normalize(scene.viewpos.xyz - vWorldPos);

    // Computed once and applied to every light's contribution, matching mesh3d.frag:
    // the engine keeps a single shadow map for one casting light, so a second light
    // gets no shadow term of its own. The geometric normal goes in even when a normal
    // map is bound, because the bias is about how the real surface faces the light
    // rather than about its bumps. 1.0 means fully lit.
    float shadowing = 1.0;
    if (scene.shadowlightpos.w > 0.0) {
        shadowing = 1.0 - calculateShadow(normalize(vNormal));
    }

    // Reflectance at normal incidence: 0.04 for dielectrics, the albedo itself for
    // metals, blended by the metallic value.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);
    for (int i = 0; i < scene.lightnum; ++i) {
        if ((scene.enabledlights & (1 << i)) == 0) continue;

        int lightType = scene.lights[i].type;

        if (lightType == 0) {
            ambientSum += scene.lights[i].ambient.rgb;
            continue;
        }

        vec3 L;
        vec3 radiance;

        if (lightType == 1) {
            L = normalize(-scene.lights[i].direction);
            radiance = scene.lights[i].diffuse.rgb * PI;
        } else {
            L = normalize(scene.lights[i].position - vWorldPos);
            float distance = length(scene.lights[i].position - vWorldPos);
            float attenuation = 1.0 / (scene.lights[i].constant +
                    scene.lights[i].linear * distance +
                    scene.lights[i].quadratic * (distance * distance));
            radiance = scene.lights[i].diffuse.rgb * PI * attenuation;

            if (lightType == 3) {
                float theta = dot(L, normalize(-scene.lights[i].direction));
                float epsilon = scene.lights[i].cutOff - scene.lights[i].outerCutOff;
                float intensity = clamp((theta - scene.lights[i].outerCutOff) / epsilon, 0.0, 1.0);
                radiance *= intensity;
            }
        }

        vec3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        // Only the direct term is shadowed. The ambient one is accumulated separately
        // below and stays untouched, which is what keeps a shadowed surface from
        // going pure black - the same rule mesh3d.frag follows.
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadowing;
    }

    // The global ambient only stands in when the scene has no lights at all, which
    // is the same rule the non-PBR shader follows.
    vec3 ambientLight = ambientSum;
    if (dot(ambientLight, ambientLight) < 0.001 && scene.lightnum == 0) {
        ambientLight = scene.globalambientcolor.rgb;
    }
    vec3 ambient = ambientLight * albedo * ao;

    vec3 color = ambient + Lo;

    // Reinhard tonemap and gamma, applied unconditionally here - unlike the non-PBR
    // shader, where both hang off scene flags.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
