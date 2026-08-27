#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

//G-Buffer Textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

//Light Variables
uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 viewPos;

//Shadow Variables
uniform mat4 lightMatrix;
uniform sampler2D shadowMap;
uniform vec3 shadowLightPos;

//Scene UBO matches GlistEngine exactly
struct Fog {
    vec3 color;
    float linearStart;
    float linearEnd;
    float density;
    float gradient;
    int mode;
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

// Single-Pass Screen Space Directional Occlusion (Fake SSAO/Cavity)
float calculateAO(vec2 texCoords, vec3 fragPos, vec3 normal) {
    vec2 texel = 1.0 / vec2(textureSize(gPosition, 0));
    float occlusion = 0.0;
    
    vec2 offsets[8] = vec2[](
        vec2( 1.5,  0.0), vec2(-1.5,  0.0), vec2( 0.0,  1.5), vec2( 0.0, -1.5),
        vec2( 2.0,  2.0), vec2(-2.0, -2.0), vec2( 2.0, -2.0), vec2(-2.0,  2.0)
    );
    
    float radius = 1.5; // Max distance in world space
    
    for(int i = 0; i < 8; i++) {
        vec3 samplePos = texture(gPosition, texCoords + offsets[i] * texel).rgb;
        if (length(samplePos) < 0.1) continue; // Skip skybox
        
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
    
    vec4 normData = texture(gNormal, TexCoords);
    vec3 Normal = normData.rgb;
    float specIntensity = normData.a;
    
    vec4 Albedo = texture(gAlbedo, TexCoords);
    // Removed Gamma Correction here to match the Forward renderer perfectly!
    
    // Discard empty skybox areas
    if(length(Normal) < 0.1) {
        FragColor = Albedo;
        return;
    }
    
    vec3 norm = normalize(Normal);
    vec3 vDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos); 
    
    // Evaluate Fake SSAO for corner and edge depth
	float rawAO = calculateAO(TexCoords, FragPos, norm);
	float aoFactor = mix(1.0, rawAO, 0.3);
    
    // Evaluate Hemispheric Ambient (Up-facing is brighter than down-facing)
    float hemi = (norm.y + 1.0) * 0.5;
    float hemiFactor = mix(0.4, 1.0, hemi);
    
    //Lighting
	vec3 totalAmbient = globalambientcolor.rgb * Albedo.rgb * aoFactor * hemiFactor;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    
    for (int i = 0; i < lightnum; i++) {
        if ((enabledlights & (1 << i)) == 0) continue;
        
        Light light = lights[i];
        
        if (light.type == 0) {
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
        } 
        else if (light.type == 1) {
            vec3 lDir = normalize(-light.direction);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = 0.0;
            if (objShininess > 0.0) {
                spec = pow(max(dot(vDir, reflectDir), 0.0), objShininess);
            }
            
				totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
				totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb;
				totalSpecular += light.specular.rgb * spec * specIntensity;
        }
        else if (light.type == 2) {
			// Point Light
            vec3 lDir = normalize(light.position - FragPos);
            float distance = length(light.position - FragPos);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = 0.0;
            if (objShininess > 0.0) {
                spec = pow(max(dot(vDir, reflectDir), 0.0), objShininess);
            }
            
            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
            
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
			totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb;
			totalSpecular += light.specular.rgb * spec * specIntensity;
        }
        else if (light.type == 3) {
            // Spot Light
            vec3 lDir = normalize(light.position - FragPos);
            float distance = length(light.position - FragPos);
            float diff = max(dot(norm, lDir), 0.0);
            vec3 reflectDir = reflect(-lDir, norm);
            float spec = 0.0;
            if (objShininess > 0.0) {
                spec = pow(max(dot(vDir, reflectDir), 0.0), objShininess);
            }
            
            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
            float theta = dot(lDir, normalize(-light.direction));
            float epsilon = (light.cutOff - light.outerCutOff);
            float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
            
            totalAmbient += light.ambient.rgb * Albedo.rgb * aoFactor * hemiFactor;
			totalDiffuse += light.diffuse.rgb * diff * Albedo.rgb;
			totalSpecular += light.specular.rgb * spec * specIntensity;
        }
    }
    
    //Shadow Blending
    vec4 fragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);
    float shadowing = 1.0 - calculateShadow(fragPosLightSpace, FragPos, norm);
    
    totalDiffuse *= shadowing;
    totalSpecular *= shadowing;
    
    vec4 result = vec4(totalAmbient + totalDiffuse + totalSpecular, Albedo.a);
    
    //Post-Processing 
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