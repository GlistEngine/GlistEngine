#version 450
// Vulkan classic-material G-buffer writer. Its vertex stage is the existing
// VULKAN3D build of color_vert.glsl, so model/instance transforms and the fixed
// gVertex layout stay identical to forward rendering.

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vColor;
layout(location = 3) in vec2 vTexCoords;
layout(location = 4) in vec3 vTangent;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;

layout(set = 1, binding = 0) uniform sampler2D diffusemap;
layout(set = 1, binding = 1) uniform sampler2D specularmap;
layout(set = 1, binding = 2) uniform sampler2D normalmap;

layout(constant_id = 0) const int GVK_CUTOUT = 1;

// Kept identical to color_vert/color_frag. The current scene descriptor sets
// were allocated from that vertex+fragment layout and Vulkan requires the stage
// flags to match as well as the binding type. lightnum is only used by the guard
// below; a negative count is invalid engine state and never occurs normally.
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
    vec4 shadowlightpos;
    int lightnum;
    int enabledlights;
    int softshadows;
    int flags;
    vec4 fogcolor;
    vec4 fogparams;
    Light lights[8];
} scene;

layout(push_constant) uniform Push {
    mat4 model;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    // x shininess, y map bitfield (1 diffuse, 2 specular, 4 normal), zw tiling.
    vec4 misc;
} pc;

bool useDiffuseMap() { return (int(pc.misc.y + 0.5) & 1) != 0; }
bool useSpecularMap() { return (int(pc.misc.y + 0.5) & 2) != 0; }
bool useNormalMap() { return (int(pc.misc.y + 0.5) & 4) != 0; }

void main() {
	if (scene.lightnum < 0) discard;
    vec4 albedo = pc.diffuse;
    if (useDiffuseMap()) {
        vec4 sampled = texture(diffusemap, vTexCoords);
        if (GVK_CUTOUT != 0 && sampled.a < 0.5) discard;
        albedo = sampled * pc.diffuse;
    }
    gAlbedo = albedo * vec4(vColor, 1.0);

    vec3 normal = normalize(vNormal);
    if (useNormalMap()) {
        vec3 tangent = normalize(vTangent - dot(vTangent, normal) * normal);
        vec3 bitangent = cross(normal, tangent);
        vec3 tangentNormal = texture(normalmap, vTexCoords).rgb * 2.0 - 1.0;
        normal = normalize(mat3(tangent, bitangent, normal) * tangentNormal);
    }

    float specular = useSpecularMap()
            ? texture(specularmap, vTexCoords).r
            : (pc.specular.r + pc.specular.g + pc.specular.b) / 3.0;
    gPosition = vec4(vFragPos, pc.misc.x);
    gNormal = vec4(normal, specular);
}
