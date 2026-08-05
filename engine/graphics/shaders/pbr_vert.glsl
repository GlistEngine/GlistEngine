#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 6) in mat4 instanceModel;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec4 FragPosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform int useInstancing;
uniform vec2 textureTiling;
uniform mat4 lightMatrix;

void main() {
	mat4 modelMatrix = useInstancing == 1 ? model * instanceModel : model;
    TexCoords = aTexCoords * textureTiling;
	WorldPos = vec3(modelMatrix * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
	FragPosLightSpace = lightMatrix * vec4(WorldPos, 1.0);

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}
