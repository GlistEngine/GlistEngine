#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

struct Light {
	int type; //0-ambient, 1-directional, 2-point, 3-spot
    vec3 position;
    vec3 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};
layout(std140) uniform Lights {
	int lightnum;
	int enabledlights;
	vec4 globalambientcolor;
	Light lights[GLIST_MAX_LIGHTS];
};

uniform vec3 camPos;

// Texture availability flags
uniform int hasAlbedoMap;
uniform int hasNormalMap;
uniform int hasMetallicMap;
uniform int hasRoughnessMap;
uniform int hasAOMap;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    // material properties - use defaults when maps aren't available
    vec3 albedo = hasAlbedoMap > 0 ? pow(texture(albedoMap, TexCoords).rgb, vec3(2.2)) : vec3(0.5);
    float metallic = hasMetallicMap > 0 ? texture(metallicMap, TexCoords).r : 0.0;
    float roughness = hasRoughnessMap > 0 ? texture(roughnessMap, TexCoords).r : 0.5;
    float ao = hasAOMap > 0 ? texture(aoMap, TexCoords).r : 1.0;

    // input lighting data
    vec3 N = hasNormalMap > 0 ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);
    for(int i = 0; i < lightnum; ++i) {
        // Check if light is enabled
        if ((enabledlights & (1 << i)) == 0) continue;

        int lightType = lights[i].type;

        // Ambient light - just accumulate ambient color
        if (lightType == 0) {
            ambientSum += lights[i].ambient.rgb;
            continue;
        }

        // Calculate light direction and radiance based on type
        vec3 L;
        vec3 radiance;

        if (lightType == 1) {
            // Directional light - no attenuation
            L = normalize(-lights[i].direction);
            radiance = lights[i].diffuse.rgb * PI;
        } else {
            // Point light (2) and Spot light (3)
            L = normalize(lights[i].position - WorldPos);
            float distance = length(lights[i].position - WorldPos);
            float attenuation = 1.0 / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * (distance * distance));
            radiance = lights[i].diffuse.rgb * PI * attenuation;

            // Spot light cone
            if (lightType == 3) {
                float theta = dot(L, normalize(-lights[i].direction));
                float epsilon = lights[i].cutOff - lights[i].outerCutOff;
                float intensity = clamp((theta - lights[i].outerCutOff) / epsilon, 0.0, 1.0);
                radiance *= intensity;
            }
        }

        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting from accumulated ambient lights
    // Only use globalambientcolor when no lights are active (matches color_frag behavior)
    vec3 ambientLight = ambientSum;
    if (dot(ambientLight, ambientLight) < 0.001 && lightnum == 0) {
        ambientLight = globalambientcolor.rgb;
    }
    vec3 ambient = ambientLight * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
