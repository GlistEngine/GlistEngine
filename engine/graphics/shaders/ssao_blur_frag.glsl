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

const int BLUR_RADIUS = 2;
// Must match the sky cutoff in the SSAO pass
const float SKY_DEPTH = 0.9999;
// Gaussian weights for offsets 0, 1 and 2 texels, normalized by the loop itself
const float SPATIAL_WEIGHT[3] = float[3](1.0, 0.8, 0.4);
// How fast a depth difference kills a neighbor, relative to the center depth. Mild on
// purpose: a floor at a grazing angle has a steep gradient between neighbors already,
// and rejecting those would leave its noise unfiltered.
const float DEPTH_SHARPNESS = 30.0;

// Linearize non-linear depth buffer value to view-space distance
float linearizeDepth(float d) {
    return nearClip * farClip / (farClip - d * (farClip - nearClip));
}

void main() {
    vec2 texelSize = 1.0 / screenSize;
    vec3 fragColor = texture(colorTexture, TexCoords).rgb;
    float centerDepth = texture(depthTexture, TexCoords).r;

    // Skip blur for sky
    if (centerDepth >= SKY_DEPTH) {
        FragColor = vec4(fragColor, 1.0);
        return;
    }

    float centerLinearDepth = linearizeDepth(centerDepth);

    // Symmetric bilateral blur. The AO pass rotates its disk per pixel, so the noise is
    // high frequency and a 5x5 kernel resolves it.
    float aoSum = 0.0;
    float weightSum = 0.0;

    for (int x = -BLUR_RADIUS; x <= BLUR_RADIUS; x++) {
        for (int y = -BLUR_RADIUS; y <= BLUR_RADIUS; y++) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float sampleAO = texture(aoTexture, TexCoords + offset).r;
            float sampleDepth = texture(depthTexture, TexCoords + offset).r;

            // Edge-aware weight using linearized depth for consistent behavior at all distances
            float sampleLinearDepth = linearizeDepth(sampleDepth);
            float depthDiff = abs(centerLinearDepth - sampleLinearDepth) / centerLinearDepth;
            float weight = SPATIAL_WEIGHT[abs(x)] * SPATIAL_WEIGHT[abs(y)];
            weight /= 1.0 + depthDiff * DEPTH_SHARPNESS;

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
