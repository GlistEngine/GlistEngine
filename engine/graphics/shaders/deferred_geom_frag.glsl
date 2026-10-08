#version 330 core

layout (location = 1) out vec4 gPosition; // GL_COLOR_ATTACHMENT1
layout (location = 2) out vec4 gNormal;   // GL_COLOR_ATTACHMENT2
layout (location = 3) out vec4 gAlbedo;   // GL_COLOR_ATTACHMENT3

// Inputs from Vertex Shader
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 incolor;

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

layout(std140) uniform Scene {
    vec4 renderColor;
    vec3 viewPos;
    mat4 viewMatrix;
    int flags;
};

uniform Material material;
uniform sampler2D texture_diffuse1;

void main() {
    // Position and Shininess
    gPosition = vec4(FragPos, material.shininess > 0.0 ? material.shininess : 32.0);
    
    // 1. Normal automation
    vec3 finalNormal = normalize(Normal);
    if (material.useNormalMap > 0) {
        vec3 mapNormal = texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0;
        finalNormal = normalize(mapNormal);
    }

    // 2. Albedo / Color automation
    vec4 baseColor = vec4(1.0);
    
    if (material.useDiffuseMap > 0) {
        vec4 texColor = texture(material.diffusemap, TexCoords);
        if (texColor.a < 0.1) discard;
        baseColor = texColor;
    } else if (material.diffuse.a > 0.0 && (material.diffuse.r + material.diffuse.g + material.diffuse.b) > 0.0) {
        baseColor = material.diffuse;
    } else {
        baseColor = vec4(0.8, 0.8, 0.8, 1.0);
    }
    
    // Vertex color and renderColor blending
    vec3 vertexColor = (incolor != vec3(0.0)) ? incolor : vec3(1.0);
    gAlbedo = baseColor * renderColor * vec4(vertexColor, 1.0);

    // 3. Specular / Roughness map automation
    float specIntensity = 0.0;
    if (material.useSpecularMap > 0) {
        specIntensity = texture(material.specularmap, TexCoords).r;
    } else {
        float matSpecAvg = (material.specular.r + material.specular.g + material.specular.b) / 3.0;
        specIntensity = (matSpecAvg > 0.0) ? matSpecAvg : clamp(1.0 - (baseColor.r * 0.5), 0.1, 0.4);
    }
    gNormal = vec4(finalNormal, specIntensity);
}