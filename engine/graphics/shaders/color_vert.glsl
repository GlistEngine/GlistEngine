#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in int aUseNormalMap;

struct Fog {
    vec3 color;
    float linearStart;
    float linearEnd;
    float density;
    float gradient;

    int mode;
};

layout(std140) uniform Scene {
    vec4 renderColor;
    vec3 viewPos;
    mat4 viewMatrix;
    uniform float ssaoBias;
    int flags;
    Fog fog;
};

uniform int aUseShadowMap;

uniform mat4 model;
uniform mat4 projection;
uniform vec3 lightPos;
uniform mat4 lightMatrix;

flat out int mUseNormalMap;
flat out int mUseShadowMap;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec4 EyePosition;

void main() {
    mUseShadowMap = aUseShadowMap;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    FragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);
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

    mat4 modelViewMatrix = viewMatrix * model;
    mat4 projectedMatrix = projection * modelViewMatrix;
    vec4 aPosVec4 = vec4(aPos, 1.0);
    gl_Position = projectedMatrix * aPosVec4;
    EyePosition = modelViewMatrix * aPosVec4;
}

