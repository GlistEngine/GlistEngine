#if GLES
#version 300 es
precision highp float;
precision highp int;
#else
#version 330 core
#endif

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;

uniform vec3 samples[32];
uniform mat4 projection;
uniform mat4 invProjection;
uniform vec2 screenSize;
uniform float ssaoRadius;
uniform float ssaoBias;
uniform float nearClip;
uniform float farClip;

vec3 getViewPos(vec2 uv) {
    float depth = texture(depthTexture, uv).r;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProjection * clipPos;
    return viewPos.xyz / viewPos.w;
}

// Linearize depth for stable range checks across all distances
float linearizeDepth(float d) {
    return nearClip * farClip / (farClip - d * (farClip - nearClip));
}

// Stable normal reconstruction using soft-weighted blend of neighbors.
// Avoids hard switches between left/right neighbors that cause jitter during rotation.
vec3 reconstructNormal(vec3 posC, vec2 uv) {
    vec2 texelSize = 1.0 / screenSize;

    vec3 posL = getViewPos(uv - vec2(texelSize.x, 0.0));
    vec3 posR = getViewPos(uv + vec2(texelSize.x, 0.0));
    vec3 posT = getViewPos(uv - vec2(0.0, texelSize.y));
    vec3 posB = getViewPos(uv + vec2(0.0, texelSize.y));

    // Soft-weight neighbors by inverse depth difference: closer-in-depth = higher weight.
    // This preserves edges (large depth jump -> low weight) without the hard binary switch
    // that caused normals to flip during camera rotation.
    float wL = 1.0 / (abs(posL.z - posC.z) + 0.0001);
    float wR = 1.0 / (abs(posR.z - posC.z) + 0.0001);
    float wT = 1.0 / (abs(posT.z - posC.z) + 0.0001);
    float wB = 1.0 / (abs(posB.z - posC.z) + 0.0001);

    vec3 dx = (wR * (posR - posC) + wL * (posC - posL)) / (wR + wL);
    vec3 dy = (wB * (posB - posC) + wT * (posC - posT)) / (wT + wB);

    vec3 normal = normalize(cross(dy, dx));
    // Ensure normal faces toward camera using view direction (more robust than Z-sign check)
    if (dot(normal, normalize(-posC)) < 0.0) normal = -normal;
    return normal;
}

void main() {
    float depth = texture(depthTexture, TexCoords).r;

    // Skip SSAO for sky / no geometry, output AO = 1.0 (no occlusion)
    if (depth >= 1.0) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 fragPos = getViewPos(TexCoords);
    vec3 normal = reconstructNormal(fragPos, TexCoords);

    // Sample noise texture (tiled 4x4)
    vec2 noiseScale = screenSize / 4.0;
    vec3 randomVec = normalize(texture(noiseTexture, TexCoords * noiseScale).xyz * 2.0 - 1.0);

    // Create TBN matrix for orienting samples along the normal
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    int kernelSize = 32;
    int validSamples = 0;
    float fragLinearDepth = linearizeDepth(depth);

    for (int i = 0; i < kernelSize; i++) {
        // Transform sample from tangent space to view space
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * ssaoRadius;

        // Project sample position to screen space
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        // Skip samples outside screen bounds
        if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0) {
            continue;
        }

        validSamples++;

        // Get actual geometry position at the projected sample location
        vec3 geometryPos = getViewPos(offset.xy);
        vec3 diff = geometryPos - fragPos;

        // Normal-based occlusion test (view-independent):
        // Check if geometry at the projected position is above the surface tangent plane.
        // dot(diff, normal) > 0 means geometry is on the normal side (above surface) = occluding.
        // On flat surfaces this is ~0 regardless of view angle, eliminating grazing-angle artifacts.
        float heightAbovePlane = dot(diff, normal);

        // Range check using linearized depth to ignore far-away geometry (e.g., sky behind edges)
        float sampleRawDepth = texture(depthTexture, offset.xy).r;
        float sampleLinearDepth = linearizeDepth(sampleRawDepth);
        float depthDiff = abs(fragLinearDepth - sampleLinearDepth);
        float rangeCheck = smoothstep(0.0, 1.0, ssaoRadius / (depthDiff + 0.0001));

        // Smooth ramp instead of hard step: small normal jitter causes small AO change, not a hard flip
        occlusion += smoothstep(0.0, ssaoBias * 2.0, heightAbovePlane) * rangeCheck;
    }

    // Divide by valid sample count to avoid edge brightening
    float ao = 1.0;
    if (validSamples > 0) {
        ao = 1.0 - (occlusion / float(validSamples));
    }

    ao = clamp(ao, 0.0, 1.0);

    // Output: R = AO factor, stored for blur pass
    FragColor = vec4(ao, 0.0, 0.0, 1.0);
}
