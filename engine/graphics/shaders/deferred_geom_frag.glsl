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
    gPosition = vec4(FragPos, material.shininess);
    
    vec3 finalNormal = normalize(Normal);
    float specIntensity = 0.0;
    
    // Albedo / Color
    if (material.useDiffuseMap > 0) {
        vec4 texColor = texture(material.diffusemap, TexCoords);
        if (texColor.a < 0.5) discard;
        gAlbedo = texColor * renderColor * vec4(incolor, 1.0);
    } else {
        // Try fallback to texture_diffuse1 if material doesn't use map explicitly but texture is bound
        // This handles cases where mesh just binds a texture
        gAlbedo = material.diffuse * renderColor * vec4(incolor, 1.0);
        
        vec4 fallbackTex = texture(texture_diffuse1, TexCoords);
        if(fallbackTex.a > 0.0) {
            // Very simple heuristic for fallback
        }
    }
    
    if (material.useSpecularMap > 0) {
        specIntensity = texture(material.specularmap, TexCoords).r;
    } else {
        specIntensity = (material.specular.r + material.specular.g + material.specular.b) / 3.0;
    }

    gNormal = vec4(finalNormal, specIntensity);
}