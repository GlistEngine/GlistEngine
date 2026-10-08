#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform mat4 projection;
uniform mat4 view;

const int NUM_SAMPLES = 16;
const float RAY_STEP = 0.1;
const float MAX_DISTANCE = 3.0;

vec3 getSampleDirection(vec3 normal, int index) {
    float angle = float(index) * (6.2831853 / float(NUM_SAMPLES));
    vec3 randDir = vec3(cos(angle), sin(angle), float(index) / float(NUM_SAMPLES));
    return normalize(normal + randDir * 0.5);
}

void main() {
    vec4 posData = texture(gPosition, TexCoords);
    vec3 fragPos = posData.rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    
    if (length(normal) < 0.1) {
        FragColor = vec4(0.0);
        return;
    }

    vec3 indirectLight = vec3(0.0);
    int validHits = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec3 rayDir = getSampleDirection(normal, i);
        vec3 currentRayPos = fragPos;

        for (float t = 0.0; t < MAX_DISTANCE; t += RAY_STEP) {
            currentRayPos += rayDir * RAY_STEP;

            vec4 clipPos = projection * view * vec4(currentRayPos, 1.0);
            vec3 ndcPos = clipPos.xyz / clipPos.w;
            vec2 sampleTexCoords = ndcPos.xy * 0.5 + 0.5;

            if (sampleTexCoords.x < 0.0 || sampleTexCoords.x > 1.0 || 
                sampleTexCoords.y < 0.0 || sampleTexCoords.y > 1.0) {
                break;
            }

            vec3 scenePos = texture(gPosition, sampleTexCoords).rgb;
            float depthDiff = length(currentRayPos - fragPos) - length(scenePos - fragPos);

            if (depthDiff > 0.01 && depthDiff < 0.3) {
                vec3 hitColor = texture(gAlbedo, sampleTexCoords).rgb;
                indirectLight += hitColor;
                validHits++;
                break;
            }
        }
    }

    if (validHits > 0) {
        indirectLight /= float(NUM_SAMPLES);
    }
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    
    FragColor = vec4(indirectLight * albedo * 1.5, 1.0);
}