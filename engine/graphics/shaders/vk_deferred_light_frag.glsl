#version 450

layout(location = 0) in vec2 vTexCoords;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D gPosition;
layout(set = 1, binding = 1) uniform sampler2D gNormal;
layout(set = 1, binding = 2) uniform sampler2D gAlbedo;
layout(set = 2, binding = 0) uniform sampler2D shadowmap;

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

const int ENABLE_FOG_FLAG = 1 << 1;
const int ENABLE_GAMMA_FLAG = 1 << 2;
const int ENABLE_HDR_FLAG = 1 << 3;

float shadowAmount(vec3 fragPos, vec3 normal) {
    if (scene.shadowlightpos.w <= 0.0) return 0.0;
    vec4 lightSpace = scene.lightmatrix * vec4(fragPos, 1.0);
    vec3 projected = lightSpace.xyz / lightSpace.w;
    projected.xy = projected.xy * 0.5 + 0.5;
    if (projected.z > 1.0) return 0.0;

    vec3 lightDir = normalize(scene.shadowlightpos.xyz - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    vec2 texel = 1.0 / vec2(textureSize(shadowmap, 0));
    int radius = scene.softshadows != 0 ? 2 : 1;
    float shadow = 0.0;
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            float depth = texture(shadowmap, projected.xy + vec2(x, y) * texel).r;
            shadow += projected.z - bias > depth ? 1.0 : 0.0;
        }
    }
    float width = float(radius * 2 + 1);
    return shadow / (width * width);
}

float fogVisibility(float distanceToCamera) {
    if (scene.fogcolor.w < 0.5) {
        float fogLength = max(scene.fogparams.w - scene.fogparams.z, 1e-5);
        return clamp((scene.fogparams.w - distanceToCamera) / fogLength, 0.0, 1.0);
    }
    return clamp(exp(-pow(distanceToCamera * scene.fogparams.x,
            scene.fogparams.y)), 0.0, 1.0);
}

void main() {
    vec4 positionData = texture(gPosition, vTexCoords);
    vec4 normalData = texture(gNormal, vTexCoords);
    vec4 albedo = texture(gAlbedo, vTexCoords);
    vec3 fragPos = positionData.rgb;

    // A cleared normal marks pixels where the geometry pass wrote no surface.
    if (length(normalData.rgb) < 0.1) {
        outColor = albedo;
        return;
    }

    vec3 normal = normalize(normalData.rgb);
    vec3 viewDir = normalize(scene.viewpos.xyz - fragPos);
    float shininess = positionData.a > 1.0 ? positionData.a : 32.0;
    float specularIntensity = normalData.a > 0.01 ? normalData.a : 0.35;
    float hemi = mix(0.4, 1.0, normal.y * 0.5 + 0.5);
    float visibleToShadowLight = 1.0 - shadowAmount(fragPos, normal);

    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    bool hasLight = false;

    for (int i = 0; i < scene.lightnum && i < 8; ++i) {
        if ((scene.enabledlights & (1 << i)) == 0) continue;
        hasLight = true;
        Light light = scene.lights[i];

        if (light.type == 0) {
            ambient += light.ambient.rgb * albedo.rgb * hemi;
            continue;
        }

        vec3 lightDir = light.type == 1
                ? normalize(-light.direction)
                : normalize(light.position - fragPos);
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 reflected = reflect(-lightDir, normal);
        float specularFactor = pow(max(dot(viewDir, reflected), 0.0), shininess);
        float attenuation = 1.0;
        float intensity = 1.0;

        if (light.type == 2 || light.type == 3) {
            float distanceToLight = length(light.position - fragPos);
            attenuation = 1.0 / max(light.constant + light.linear * distanceToLight
                    + light.quadratic * distanceToLight * distanceToLight, 1e-5);
        }
        if (light.type == 3) {
            float theta = dot(lightDir, normalize(-light.direction));
            float inner = cos(radians(light.cutOff));
            float outer = cos(radians(light.outerCutOff));
            intensity = clamp((theta - outer) / max(inner - outer, 0.0001), 0.0, 1.0);
        }

        // A shadow map belongs to one scene light. Do not let that light's
        // occluders suppress unrelated point/spot lights in the same pixel.
        float directVisibility = distance(light.position, scene.shadowlightpos.xyz) < 0.001
                ? visibleToShadowLight
                : 1.0;
        // Match the OpenGL deferred shader exactly: point-light ambient is not
        // distance attenuated, while a spotlight's ambient follows its cone and
        // distance. The spotlight's direct terms carry the same 1.8 boost used by
        // that shader so its authored intensity does not change between backends.
        float ambientScale = light.type == 3 ? attenuation * intensity : 1.0;
        float directScale = light.type == 3
                ? attenuation * intensity * 1.8
                : (light.type == 2 ? attenuation : 1.0);
        ambient += light.ambient.rgb * albedo.rgb * hemi * ambientScale;
        diffuse += light.diffuse.rgb * diffuseFactor * albedo.rgb
                * directScale * directVisibility;
        specular += light.specular.rgb * specularFactor * specularIntensity
                * directScale * directVisibility;
    }

    if (!hasLight) ambient = scene.globalambientcolor.rgb * albedo.rgb * hemi;
    vec4 result = vec4(ambient + diffuse + specular, albedo.a);

    if ((scene.flags & ENABLE_FOG_FLAG) != 0) {
        float visibility = fogVisibility(length(scene.viewpos.xyz - fragPos));
        result = mix(vec4(scene.fogcolor.rgb, 1.0), result, visibility);
    }
    if ((scene.flags & ENABLE_HDR_FLAG) != 0) {
        result.rgb = result.rgb / (result.rgb + vec3(1.0));
    }
    if ((scene.flags & ENABLE_GAMMA_FLAG) != 0) {
        result.rgb = pow(result.rgb, vec3(1.0 / 2.2));
    }
    outColor = result;
}
