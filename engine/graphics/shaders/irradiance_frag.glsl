#if defined(GLIST_MOBILE)
#version 300 es
precision highp float;
#else
#version 330 core
#endif

in vec3 WorldPos;
out vec4 FragColor;
uniform samplerCube environmentMap;

const float PI = 3.14159265359;

// Mirror binary digits about the decimal point
float radicalInverse_VdC(int bits) {
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Randomish sequence that has pretty evenly spaced points
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 hammersley(int i, int N) {
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up = abs(N.z) < 0.999 ? vec3 (0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main() {
    // Not the normal of the cube, but the normal that we're calculating the irradiance for
    vec3 normal = normalize(WorldPos);
    vec3 N = normal;

    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);

    // ------------------------------------------------------------------------------

    const int SAMPLE_COUNT = 16384;
    float totalWeight = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, 1.0);

        // NdotH is equal to cos(theta)
        float NdotH = max(dot(N, H), 0.0);
        // With roughness == 1 in the distribution function we get 1/pi
        float D = 1.0 / PI;
        float pdf = (D * NdotH / (4.0)) + 0.0001;

        float resolution = 1024.0; // resolution of source cubemap (per face)
        // with a higher resolution, we should sample coarser mipmap levels
        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
        // as we take more samples, we can sample from a finer mipmap.
        // And places where H is more likely to be sampled (higher pdf) we
        // can use a finer mipmap, otherwise use courser mipmap.
        // The tutorial treats this portion as optional to reduce noise but I think it's
        // actually necessary for importance sampling to get the correct result
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

        float mipLevel = 0.5 * log2(saSample / saTexel);

        irradiance += textureLod(environmentMap, H, mipLevel).rgb * NdotH;
        // irradiance += texture(environmentMap, H).rgb * NdotH;
        totalWeight += NdotH;
    }
    irradiance = (PI * irradiance) / totalWeight;

    // irradiance = texture(environmentMap, normal).rgb;
    FragColor = vec4(irradiance, 1.0);
}