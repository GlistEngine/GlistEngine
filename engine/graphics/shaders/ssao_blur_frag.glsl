#if GLES
#version 300 es
precision highp float;
precision highp int;
#else
#version 330 core
#endif

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D colorTexture;  // Original scene color
uniform sampler2D aoTexture;     // Raw AO from SSAO pass
uniform sampler2D depthTexture;  // Scene depth for edge-aware blur
uniform vec2 screenSize;
uniform float ssaoStrength;
uniform int debugMode;
uniform float nearClip;
uniform float farClip;

// Linearize non-linear depth buffer value to view-space distance
float linearizeDepth(float d) {
    return nearClip * farClip / (farClip - d * (farClip - nearClip));
}

void main() {
    vec2 texelSize = 1.0 / screenSize;
    vec3 fragColor = texture(colorTexture, TexCoords).rgb;
    float centerDepth = texture(depthTexture, TexCoords).r;

    // Skip blur for sky
    if (centerDepth >= 1.0) {
        FragColor = vec4(fragColor, 1.0);
        return;
    }

    float centerLinearDepth = linearizeDepth(centerDepth);

    // Bilateral blur: 6x6 box blur weighted by depth similarity
    // Larger than the 4x4 noise tile for extra smoothing
    float aoSum = 0.0;
    float weightSum = 0.0;

    for (int x = -3; x <= 2; x++) {
        for (int y = -3; y <= 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float sampleAO = texture(aoTexture, TexCoords + offset).r;
            float sampleDepth = texture(depthTexture, TexCoords + offset).r;

            // Edge-aware weight using linearized depth for consistent behavior at all distances
            float sampleLinearDepth = linearizeDepth(sampleDepth);
            float depthDiff = abs(centerLinearDepth - sampleLinearDepth) / centerLinearDepth;
            float weight = 1.0 / (1.0 + depthDiff * 100.0);

            aoSum += sampleAO * weight;
            weightSum += weight;
        }
    }

    float ao = aoSum / weightSum;
    ao = mix(1.0, ao, ssaoStrength);

    if (debugMode > 0) {
        FragColor = vec4(vec3(ao), 1.0);
    } else {
        FragColor = vec4(fragColor * ao, 1.0);
    }
}
