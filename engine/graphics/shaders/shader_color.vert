#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in int aUseNormalMap;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
flat out int mUseNormalMap;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    mUseNormalMap = aUseNormalMap;
    
    if (aUseNormalMap > 0) {
	    mat3 normalMatrix = transpose(inverse(mat3(model)));
	    vec3 T = normalize(normalMatrix * aTangent);
	    vec3 N = normalize(normalMatrix * aNormal);
	    T = normalize(T - dot(T, N) * N);
	    vec3 B = cross(N, T);
	    
	    mat3 TBN = transpose(mat3(T, B, N));    
	    TangentLightPos = TBN * lightPos;
	    TangentViewPos  = TBN * viewPos;
	    TangentFragPos  = TBN * FragPos;
    }
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

