#if GLES
#version 300 es
precision highp float;
precision highp int;
#else
#version 330 core
#endif

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    sampler2D diffusemap;
    sampler2D specularmap;
    sampler2D normalMap;
    int useDiffuseMap;
    int useSpecularMap;
    int useNormalMap;
};

struct Light {
    int type; //0-ambient, 1-directional, 2-point, 3-spot
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

struct Fog {
    vec3 color;
    float linearStart;
    float linearEnd;
    float density;
    float gradient;

    int mode;
};

uniform Material material;
layout(std140) uniform Lights {
    int lightnum;
    int enabledlights;
    vec4 globalambientcolor;
    Light lights[GLIST_MAX_LIGHTS];
};

int ENABLE_SSAO_FLAG = 1 << 0;
int ENABLE_FOG_FLAG = 1 << 1;
int ENABLE_GAMMA_FLAG = 1 << 2;
int ENABLE_HDR_FLAG = 1 << 3;
int ENABLE_SOFT_SHADOWS_FLAG = 1 << 4;

layout(std140) uniform Scene {
    vec4 renderColor;
    vec3 viewPos;
    mat4 viewMatrix;
    int flags;
    Fog fog;
};

uniform mat4 projection;

uniform sampler2D shadowMap;
uniform vec3 shadowLightPos;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec4 EyePosition;
in vec3 incolor;
in mat3 TBN;

flat in int mUseShadowMap;

out vec4 FragColor;

float calculateShadow(vec4 fragPosLightSpace, bool softShadows) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(shadowLightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = vec2(1.0, 1.0) / vec2(textureSize(shadowMap, 0));

    if (softShadows) {
        // 5x5 PCF for softer shadows
        for(int x = -2; x <= 2; ++x) {
            for(int y = -2; y <= 2; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 25.0;
    } else {
        // Standard 3x3 PCF
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }

    if(projCoords.z > 1.0) shadow = 0.0;
    return shadow;
}

vec4 calcAmbLight(Light light, vec4 materialAmbient) {
    vec4 ambient = light.ambient * materialAmbient;
    return ambient;
}

vec4 calcDirLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular) {
    vec3 lightDir = normalize(-light.direction);
    float diff;
    float spec;
    if (material.useNormalMap > 0) {
        diff = max(dot(lightDir, normal), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    vec4 ambient = light.ambient * materialAmbient;
    vec4 diffuse = light.diffuse * vec4(diff) * materialDiffuse;
    vec4 specular = light.specular * vec4(spec) * materialSpecular;
    if (mUseShadowMap > 0) {
        diffuse *= vec4(shadowing);
        specular *= vec4(shadowing);
    }
    return (ambient + diffuse + specular);
}

vec4 calcPointLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){
    vec3 lightDir;
    float distance;
    float diff;
    float spec;
    if (material.useNormalMap > 0) {
        vec3 tangentLightPos = TBN * light.position;
        lightDir = normalize(tangentLightPos - TangentFragPos);
        distance = length(tangentLightPos - TangentFragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        lightDir = normalize(light.position - FragPos);
        distance = length(light.position - FragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    vec4 ambient = light.ambient * materialAmbient;
    vec4 diffuse = light.diffuse * diff * materialDiffuse;
    vec4 specular = light.specular * spec * materialSpecular;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    if (mUseShadowMap > 0) {
        diffuse  *= shadowing;
        specular *= shadowing;
    }
    return (ambient + diffuse + specular);
}

vec4 calcSpotLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){
    vec3 lightDir;
    float distance;
    float diff;
    float spec;
    if (material.useNormalMap > 0) {
        vec3 tangentLightPos = TBN * light.position;
        lightDir = normalize(tangentLightPos - TangentFragPos);
        distance = length(tangentLightPos - TangentFragPos);
        diff = max(dot(lightDir, normal), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        lightDir = normalize(light.position - FragPos);
        distance = length(light.position - FragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    vec4 ambient  = light.ambient  * materialAmbient;
    vec4 diffuse  = light.diffuse  * diff * materialDiffuse;
    vec4 specular = light.specular * spec * materialSpecular;

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    ambient  *= attenuation;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;
    if (mUseShadowMap > 0) {
        diffuse *= shadowing;
        specular *= shadowing;
    }
    return (ambient + diffuse + specular);
}

float getFogVisibility(Fog fog, float distance) {
    float visibility = 0.0;
    if(fog.mode == 0) { // linear
        float fogLength = fog.linearEnd - fog.linearStart;
        visibility = (fog.linearEnd - distance) / fogLength;
    } else if(fog.mode == 1) { // exp
        visibility = exp(-pow((distance * fog.density), fog.gradient));
    }

    visibility = clamp(visibility, 0.0, 1.0);
    return visibility;
}

void main() {
    vec4 result = vec4(0.0);
    vec3 norm;
    if (material.useNormalMap > 0) {
        norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space
    } else {
        norm = normalize(Normal);
    }
    vec3 viewDir;
    if (material.useNormalMap > 0) {
        viewDir = normalize(TangentViewPos - TangentFragPos);
    } else {
        viewDir = normalize(viewPos - FragPos);
    }
    vec4 materialAmbient;
    vec4 materialDiffuse;
    if (material.useDiffuseMap > 0) {
        materialAmbient = texture(material.diffusemap, TexCoords).rgba;
        materialDiffuse = texture(material.diffusemap, TexCoords).rgba;
    } else {
        materialAmbient = material.ambient;
        materialDiffuse = material.diffuse;
    }
    if (material.useDiffuseMap > 0 && materialDiffuse.a < 0.5) {
        discard;
    }
    vec4 materialSpecular;
    if (material.useSpecularMap > 0) {
        materialSpecular = texture(material.specularmap, TexCoords).rgba;
    }
    else {
        materialSpecular = material.specular;
    }
    float shadowing;
    if (mUseShadowMap > 0) {
        bool softShadows = (flags & ENABLE_SOFT_SHADOWS_FLAG) > 0;
        shadowing = 1.0 - calculateShadow(FragPosLightSpace, softShadows);
    }
    bool haslight = false;
    for (int i = 0; i < lightnum; i++) {
        if ((enabledlights & (1 << i)) == 0) {
            continue;
        }
        if (lights[i].type == 0) {
            result += calcAmbLight(lights[i], materialAmbient);
        } else if (lights[i].type == 1) {
            result += calcDirLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
        } else if (lights[i].type == 2) {
            result += calcPointLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
        } else if (lights[i].type == 3) {
            result += calcSpotLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
        } else {
            return;
        }
        haslight = true;
    }
    if (!haslight) {
        result = globalambientcolor * materialAmbient;
    }

    FragColor = result * renderColor * vec4(incolor, 1.0);

    if((flags & ENABLE_FOG_FLAG) > 0) {
        float distance = abs(EyePosition.z / EyePosition.w);
        FragColor = mix(vec4(fog.color, 1.0), FragColor, getFogVisibility(fog, distance));
    }

    // HDR tone mapping (Reinhard)
    if((flags & ENABLE_HDR_FLAG) > 0) {
        vec3 hdrColor = FragColor.rgb;
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
        FragColor = vec4(mapped, FragColor.a);
    }

    // Gamma correction
    if((flags & ENABLE_GAMMA_FLAG) > 0) {
        float gamma = 2.2;
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
    }
}