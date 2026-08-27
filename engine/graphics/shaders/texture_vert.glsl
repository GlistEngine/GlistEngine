#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 worldPos = model * vec4(aPos, 1.0);
	FragPos = worldPos.xyz;
	
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	Normal = normalMatrix * aNormal;
	
    TexCoords = aTexCoords;
    gl_Position = projection * view * worldPos;
}
