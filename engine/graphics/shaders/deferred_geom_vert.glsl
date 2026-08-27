#version 330 core

// Input vertex data
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec3 color;
layout (location = 6) in mat4 instanceModel;

// Variables to pass to the FS
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 incolor;

// Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int useInstancing;
uniform vec2 textureTiling;

void main() {
    mat4 modelMatrix = useInstancing == 1 ? model * instanceModel : model;
    
    vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Default texture tiling if not provided
    vec2 tiling = (textureTiling.x == 0.0 && textureTiling.y == 0.0) ? vec2(1.0, 1.0) : textureTiling;
    TexCoords = aTexCoords * tiling;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    Normal = normalMatrix * aNormal;
    incolor = color;
    
    gl_Position = projection * view * worldPos;
}