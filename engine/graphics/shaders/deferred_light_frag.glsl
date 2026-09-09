#version 330 core

uniform sampler2D ssgiTexture;
uniform int useSSGI;

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 viewPos;

uniform mat4 lightMatrix;
uniform sampler2D shadowMap;
uniform vec3 shadowLightPos;

struct Fog {
    vec3 color;
    float linearStart;
    float linearEnd;
    float density;
    float gradient;
    int mode;
};

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

layout(std140) uniform Lights {
    int lightnum;
    int enabledlights;
    vec4 globalambientcolor;
    Light lights[GLIST_MAX_LIGHTS];
};

layout(std140) uniform Scene {
    vec4 renderColor;
    vec3 viewPosUBO;
    mat4 viewMatrix;
    int flags;
    Fog fog;
};

int ENABLE_FOG_FLAG = 1 << 1;
int ENABLE_GAMMA_FLAG = 1 << 2;
int ENABLE_HDR_FLAG = 1 << 3;
int ENABLE_SOFT_SHADOWS_FLAG = 1 << 4;

float calculateShadow(vec4 fragPosLightSpace, vec3 FragPos, vec3 Normal) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(shadowLightPos - FragPos); 
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = vec2(1.0, 1.0) / vec2(textureSize(shadowMap, 0));

    bool softShadows = (flags & ENABLE_SOFT_SHADOWS_FLAG) > 0;

    if (softShadows) {
        for(int x = -2; x <= 2; ++x) {
            for(int y = -2; y <= 2; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 25.0;
    } else {
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    }
    return shadow;
}

float calculateAO(vec2 texCoords, vec3 fragPos, vec3 normal) {
    vec2 texel = 1.0 / vec2(textureSize(gPosition, 0));
    float occlusion = 0.0;
    
    vec2 offsets[8] = vec2[](
        vec2( 1.5,  0.0), vec2(-1.5,  0.0), vec2( 0.0,  1.5), vec2( 0.0, -1.5),
        vec2( 2.0,  2.0), vec2(-2.0, -2.0), vec2( 2.0, -2.0), vec2(-2.0,  2.0)
    );
    
    float radius = 1.5; 
    
    for(int i = 0; i < 8; i++) {
        vec3 samplePos = texture(gPosition, texCoords + offsets[i] * texel).rgb;
        if (length(samplePos) < 0.1) continue; 
        
        vec3 diff = samplePos - fragPos;
        float dist = length(diff);
        
        if (dist > 0.02 && dist < radius) {
            vec3 sampleDir = diff / dist;
            float angle = max(dot(normal, sampleDir), 0.0);
            float attenuation = smoothstep(radius, 0.02, dist);
            occlusion += angle * attenuation;
        }
    }
    return clamp(1.0 - (occlusion / 4.0), 0.0, 1.0);
}

void main() {
    vec4 posData = texture(gPosition, TexCoords);
    vec3 FragPos = posData.rgb;
    float objShininess = posData.a;
    float shininess = (objShininess > 1.0) ? objShininess : 32.0;
    
    vec4 normData = texture(gNormal, TexCoords);
    vec3 Normal = normData.rgb;
    float specIntensity = normData.a;
    float effectiveSpecIntensity = (specIntensity > 0.01) ? specIntensity : 0.35;
    
    vec4 Albedo = texture(gAlbedo, TexCoords);
    
    if(length(Normal) < 0.1) {
        FragColor = Albedo;
        return;
    }
    
    vec3 norm = normalize(Normal);
    vec3 vDir = normalize(viewPos - FragPos);
    
    float aoFactor = 1.0; 
    float hemi = (norm.y + 1.0) * 0.5;
    float hemiFactor = mix(0.4, 1.0, hemi);
    
    vec3 ssgiColor = vec3(0.0);
    if (useSSGI == 1) {
        ssgiColor = texture(ssgiTexture, TexCoords).rgb;
    }
    
    vec3 totalAmbient = (globalambientcolor.rgb + ssgiColor) * Albedo.rgb * aoFactor * hemiFactor;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    
    bool haslight = false;
    for (int i = 0; i < lightnum; i++) {
        if ((enabledlights & (1 << i)) == 0) continue;
        haslight = true;
        
        Light light = lights[i];
        
        if (light.type == 0) {
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
        } 
        else if (light.type == 1) { 
            vec3 lDir = normalize(-light.direction);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = pow(max(dot(vDir, reflectDir), 0.0), shininess);
            
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
            totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb * aoFactor;
            totalSpecular += light.specular.rgb * spec * effectiveSpecIntensity * aoFactor; 
        }
        else if (light.type == 2) { 
            vec3 lDir = normalize(light.position - FragPos);
            float distance = length(light.position - FragPos);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = pow(max(dot(vDir, reflectDir), 0.0), shininess);
            
            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
            
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor * attenuation;
            totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb * attenuation * aoFactor;
            totalSpecular += light.specular.rgb * spec * effectiveSpecIntensity * attenuation * aoFactor;
        }
        else if (light.type == 3) { 
            vec3 lDir = normalize(light.position - FragPos);
            float distance = length(light.position - FragPos);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = pow(max(dot(vDir, reflectDir), 0.0), shininess);
            
            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
            float theta = dot(lDir, normalize(-light.direction));
            float cutOffCos = cos(radians(light.cutOff));
            float outerCutOffCos = cos(radians(light.outerCutOff));
            float epsilon = max(cutOffCos - outerCutOffCos, 0.0001);
            float intensity = clamp((theta - outerCutOffCos) / epsilon, 0.0, 1.0);
            
            float spotIntensity = attenuation * intensity * 1.8;
            
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor * attenuation * intensity;
            totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb * spotIntensity * aoFactor;
            totalSpecular += light.specular.rgb * spec * effectiveSpecIntensity * spotIntensity * aoFactor;
        }
    }
    
    if (!haslight) {
        totalAmbient = globalambientcolor.rgb * Albedo.rgb * aoFactor * hemiFactor;
    }

    vec4 fragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);
    float shadowing = 1.0 - calculateShadow(fragPosLightSpace, FragPos, norm);
    
    totalDiffuse *= shadowing;
    totalSpecular *= shadowing;
    
    vec4 result = vec4(totalAmbient + totalDiffuse + totalSpecular, Albedo.a);
    
    if((flags & ENABLE_FOG_FLAG) > 0) {
        float distance = length(viewPos - FragPos);
        float visibility = 0.0;
        if(fog.mode == 0) {
            float fogLength = fog.linearEnd - fog.linearStart;
            visibility = (fog.linearEnd - distance) / fogLength;
        } else if(fog.mode == 1) {
            visibility = exp(-pow((distance * fog.density), fog.gradient));
        }
        visibility = clamp(visibility, 0.0, 1.0);
        result = mix(vec4(fog.color, 1.0), result, visibility);
    }

    if((flags & ENABLE_HDR_FLAG) > 0) {
        vec3 hdrColor = result.rgb;
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
        result = vec4(mapped, result.a);
    }

    if((flags & ENABLE_GAMMA_FLAG) > 0) {
        float gamma = 2.2;
        result.rgb = pow(result.rgb, vec3(1.0 / gamma));
    }
    
    FragColor = result;
}