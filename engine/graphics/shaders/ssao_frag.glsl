#if GLES
#version 300 es
precision highp float;
precision highp int;
#else
#version 330 core
#endif

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depthTexture;

uniform mat4 projection;
uniform mat4 invProjection;
uniform vec2 screenSize;
uniform float ssaoRadius;
uniform float ssaoBias;

const int SAMPLE_NUM = 16;
// Coprime with the sample count, otherwise the spiral collapses into a few rays
const float SPIRAL_TURNS = 7.0;
const float TAU = 6.28318530718;
// Screen space bounds for the sampling disk, in pixels
const float MIN_TAP_RADIUS = 2.0;
const float MAX_TAP_RADIUS = 96.0;
// The skybox is drawn at exactly 1.0, the margin only covers depth texture precision
const float SKY_DEPTH = 0.9999;

vec3 viewPosFromDepth(vec2 uv, float depth) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProjection * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 getViewPos(vec2 uv) {
    uv = clamp(uv, 0.5 / screenSize, 1.0 - 0.5 / screenSize);
    return viewPosFromDepth(uv, texture(depthTexture, uv).r);
}

// Unique rotation per pixel, so the leftover noise reads as grain instead of a grid
float interleavedGradientNoise(vec2 pos) {
    return fract(52.9829189 * fract(dot(pos, vec2(0.06711056, 0.00583715))));
}

// For each axis the two neighbors on one side are extrapolated towards the center and
// the side that predicts it best wins, so the normal never blends across a silhouette.
vec3 reconstructNormal(vec3 posC, vec2 uv, vec2 texelSize) {
    vec3 posL1 = getViewPos(uv - vec2(texelSize.x, 0.0));
    vec3 posL2 = getViewPos(uv - vec2(texelSize.x * 2.0, 0.0));
    vec3 posR1 = getViewPos(uv + vec2(texelSize.x, 0.0));
    vec3 posR2 = getViewPos(uv + vec2(texelSize.x * 2.0, 0.0));
    vec3 posD1 = getViewPos(uv - vec2(0.0, texelSize.y));
    vec3 posD2 = getViewPos(uv - vec2(0.0, texelSize.y * 2.0));
    vec3 posU1 = getViewPos(uv + vec2(0.0, texelSize.y));
    vec3 posU2 = getViewPos(uv + vec2(0.0, texelSize.y * 2.0));

    float errorL = abs(2.0 * posL1.z - posL2.z - posC.z);
    float errorR = abs(2.0 * posR1.z - posR2.z - posC.z);
    float errorD = abs(2.0 * posD1.z - posD2.z - posC.z);
    float errorU = abs(2.0 * posU1.z - posU2.z - posC.z);

    vec3 derivX = errorL < errorR ? posC - posL1 : posR1 - posC;
    vec3 derivY = errorD < errorU ? posC - posD1 : posU1 - posC;

    vec3 normal = normalize(cross(derivX, derivY));
    // A visible surface always has dot(normal, position) < 0 in view space
    if (dot(normal, posC) > 0.0) normal = -normal;
    return normal;
}

void main() {
    float depth = texture(depthTexture, TexCoords).r;

    // Skip SSAO for sky / no geometry, output AO = 1.0 (no occlusion)
    if (depth >= SKY_DEPTH) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    vec2 texelSize = 1.0 / screenSize;
    vec3 fragPos = viewPosFromDepth(TexCoords, depth);
    vec3 normal = reconstructNormal(fragPos, TexCoords, texelSize);

    // Project the world space radius to the sampling disk radius in pixels at this depth
    float viewDepth = max(-fragPos.z, 0.0001);
    float pixelsPerUnit = projection[1][1] * 0.5 * screenSize.y / viewDepth;
    float tapRadius = clamp(ssaoRadius * pixelsPerUnit, MIN_TAP_RADIUS, MAX_TAP_RADIUS);
    // The falloff has to fade over the distance the clamped taps actually cover, else a
    // radius larger than the view distance leaves every tap at full weight
    float radius = tapRadius / pixelsPerUnit;

    float angleOffset = interleavedGradientNoise(gl_FragCoord.xy) * TAU;

    // Each tap contributes its elevation above the tangent plane faded out over the
    // radius. Both factors are in [0, 1], so no single tap can drive the pixel black and
    // the estimate keeps its meaning at any scene scale.
    float occlusion = 0.0;

    for (int i = 0; i < SAMPLE_NUM; i++) {
        float alpha = (float(i) + 0.5) / float(SAMPLE_NUM);
        float angle = alpha * SPIRAL_TURNS * TAU + angleOffset;
        vec2 tapUv = TexCoords + (alpha * tapRadius) * vec2(cos(angle), sin(angle)) * texelSize;
        // Clamping instead of skipping keeps the sample count constant, and a clamped tap
        // sits next to the fragment so it reports no occlusion
        tapUv = clamp(tapUv, texelSize * 0.5, 1.0 - texelSize * 0.5);

        // Sky sits on the far plane, reconstructing it would add an occluder kilometers away
        float tapDepth = texture(depthTexture, tapUv).r;
        if (tapDepth >= SKY_DEPTH) continue;

        vec3 diff = viewPosFromDepth(tapUv, tapDepth) - fragPos;
        float dist = length(diff);
        if (dist < 0.0001) continue;

        // Bias is a slope threshold, it absorbs the error of normals rebuilt from depth
        float elevation = max(dot(diff, normal) / dist - ssaoBias, 0.0);
        float attenuation = 1.0 - clamp(dist / radius, 0.0, 1.0);
        occlusion += elevation * attenuation * attenuation;
    }

    // A fragment in a right angle has about half its taps occluded, the doubling is what
    // makes that read as a half dark crease
    float ao = 1.0 - 2.0 * occlusion / float(SAMPLE_NUM);

    // Output: R = AO factor, stored for blur pass
    FragColor = vec4(clamp(ao, 0.0, 1.0), 0.0, 0.0, 1.0);
}
