#if GLES
#version 300 es
precision highp float;
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

layout(std140) uniform Scene {
    vec4 renderColor;
    vec3 viewPos;
    mat4 viewMatrix;
    uniform float ssaoBias;
    int flags;
    Fog fog;
};

uniform mat4 projection;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
flat in int mUseNormalMap;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec4 EyePosition;

flat in int mUseShadowMap;
uniform sampler2D shadowMap;
uniform vec3 shadowLightPos;

out vec4 FragColor;

float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(shadowLightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = vec2(0.5, 0.5) / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
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
    if (mUseNormalMap > 0) {
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
    if (mUseNormalMap > 0) {
        lightDir = normalize(TangentLightPos - TangentFragPos);
        distance = length(TangentLightPos - TangentFragPos);
    } else {
        lightDir = normalize(light.position - FragPos);
        distance = length(light.position - FragPos);
    }
    float diff;
    float spec;
    if (mUseNormalMap > 0) {
        diff = max(dot(lightDir, normal), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
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
    if (mUseNormalMap > 0) {
        lightDir = normalize(TangentLightPos - TangentFragPos);
        distance = length(TangentLightPos - TangentFragPos);
    } else {
        lightDir = normalize(light.position - FragPos);
        distance = length(light.position - FragPos);
    }
    float diff;
    float spec;
    if (mUseNormalMap > 0) {
        diff = max(dot(lightDir, normal), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
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

vec4 getSSAO() {
    const int kernelSize = 16;
    const float radius = 0.1;

    vec4 fragPos = viewMatrix * vec4(FragPos, 1.0);
    vec4 clipPos = projection * fragPos;
    float ndcDepth = clipPos.z / clipPos.w;
    float depth = ((ndcDepth + 1.0) / 2.0);
    vec4 depthMap = vec4(depth, depth, depth, 1.0);

    vec3 tangentNormal = normalize(Normal * 2.0 - 1.0);
    vec3 tangentFragPos = FragPos;

    vec3 ambient = vec3(0.0);

    for (int i = 0; i < kernelSize; ++i){
        vec2 co = vec2(float(i), 0.0);
        vec2 co2 = vec2(float(i), 1.0);
        vec3 randomVec = vec3(fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453), fract(sin(dot(co2.xy, vec2(12.9898, 78.233))) * 43758.5453), 0.0);
        vec3 sampleVec = tangentFragPos + randomVec * radius;

        vec4 offset = projection * viewMatrix * vec4(sampleVec, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        float sampleDepth = depthMap.r;
        vec3 samplePos = (viewMatrix * vec4(sampleVec * sampleDepth, 1.0)).xyz;

        float occlusion = clamp(dot(Normal, normalize(samplePos - FragPos)) - ssaoBias, 0.0, 1.0);
        ambient += (1.0 - occlusion);
    }
    ambient /= float(kernelSize);
    ambient = mix(vec3(1.0), ambient, 0.5);
    return vec4(ambient, 1.0);
}


void main() {
    vec4 result = vec4(0.0);
    vec3 norm;
    if (mUseNormalMap > 0) {
        norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space
    } else {
        norm = normalize(Normal);
    }
    vec3 viewDir;
    if (mUseNormalMap > 0) {
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
        shadowing = 1.0 - calculateShadow(FragPosLightSpace);
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

    FragColor = result * renderColor;

    if((flags & ENABLE_FOG_FLAG) > 0) {
        float distance = abs(EyePosition.z / EyePosition.w);
        FragColor = mix(vec4(fog.color, 1.0), FragColor, getFogVisibility(fog, distance));
    }

    if((flags & ENABLE_SSAO_FLAG) > 0) {
        FragColor *= getSSAO();
    }
}
