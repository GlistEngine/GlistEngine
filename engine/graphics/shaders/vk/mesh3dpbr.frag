#version 450

// Cook-Torrance PBR for the Vulkan backend, ported from pbr_frag.glsl. The OpenGL
// version has no image-based lighting - no irradiance, prefilter or BRDF LUT - so
// this is a straight port of the analytic part rather than a reduced one.
//
// Unlike the mesh3d shaders, the five maps share one descriptor set instead of
// taking one set each. Vulkan only guarantees four bound sets, and scene plus five
// textures would need six; one set with five bindings needs two.

const float PI = 3.14159265359;

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in vec3 vWorldPos;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec4 vFragPosLightSpace;

layout(location = 0) out vec4 outColor;

struct Light {
    int type;
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

layout(set = 0, binding = 0) uniform Scene {
    mat4 projection;
    mat4 view;
    mat4 lightmatrix;
    vec4 viewpos;
    vec4 globalambientcolor;
    vec4 rendercolor;
    // xyz the shadow-casting light's position, w whether a shadow map is bound.
    vec4 shadowlightpos;
    int lightnum;
    int enabledlights;
    int softshadows;
    // Declared but not read here, and that is deliberate. This block is shared with
    // mesh3d, which does read them, and a uniform block has to be declared the same
    // way by every shader that binds it - leaving these out shifts the light array
    // by thirty two bytes and the PBR path reads whatever happens to sit there. The
    // symptom is one object going black while everything else looks right.
    //
    // Nothing applies them because pbr_frag.glsl does not either: fog and tone
    // mapping never reached the OpenGL PBR path, and matching it is what keeps the
    // two backends drawing the same picture.
    int flags;
    vec4 fogcolor;
    vec4 fogparams;
    Light lights[8];
} scene;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D metallicMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;
// The shadow map gets a set of its own rather than a sixth binding in the material
// set: it is per frame, not per material, and the material sets are cached by their
// five texture ids. Three sets in total still fits the four Vulkan guarantees. Like
// the maps above this binding is always filled - with the 1x1 white texture when no
// shadow map exists - and scene.shadowlightpos.w says whether to believe it.
layout(set = 2, binding = 0) uniform sampler2D shadowmap;

layout(push_constant) uniform Push {
    mat4 model;
    // maps0 = albedo, normal, metallic, roughness; maps1.x = ao. Non-zero means the
    // map is supplied; where it is not, pbr_frag.glsl's defaults are used.
    ivec4 maps0;
    ivec4 maps1;
} pc;

// Tangent basis from screen-space derivatives rather than a vertex tangent, which
// is what lets PBR work on meshes that carry no tangent attribute at all.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, vTexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(vWorldPos);
    vec3 Q2 = dFdy(vWorldPos);
    vec2 st1 = dFdx(vTexCoords);
    vec2 st2 = dFdy(vTexCoords);

    vec3 N = normalize(vNormal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// One bilinearly weighted PCF tap. The shadow map is sampled with a NEAREST filter,
// so a plain lookup snaps to whole texels and the shadow edge comes out as a
// staircase of 4096ths of the light's frustum. Reading the four texels around the
// sample point and weighting them by where inside its texel the point falls gives
// the smooth edge a comparison sampler would - the filtering is applied to the
// *results* of the depth comparisons rather than to the depths themselves, which is
// the only order that means anything: an averaged depth compared once is not a
// partial occlusion, it is a wrong occlusion.
float shadowTap(vec2 uv, float compare, vec2 texelSize) {
    vec2 texels = uv / texelSize - 0.5;
    vec2 corner = floor(texels);
    vec2 frac = texels - corner;
    vec2 base = (corner + 0.5) * texelSize;

    float d00 = texture(shadowmap, base).r;
    float d10 = texture(shadowmap, base + vec2(texelSize.x, 0.0)).r;
    float d01 = texture(shadowmap, base + vec2(0.0, texelSize.y)).r;
    float d11 = texture(shadowmap, base + texelSize).r;

    float lit00 = compare > d00 ? 0.0 : 1.0;
    float lit10 = compare > d10 ? 0.0 : 1.0;
    float lit01 = compare > d01 ? 0.0 : 1.0;
    float lit11 = compare > d11 ? 0.0 : 1.0;

    return mix(mix(lit00, lit10, frac.x), mix(lit01, lit11, frac.x), frac.y);
}

// Which way the light that filled the shadow map actually points, from the fragment
// towards it.
//
// It is not the direction of scene.shadowlightpos. gShadowMap builds the map through
// an orthographic projection - the light is a sun, parallel rays, one direction for
// the entire scene - and shadowlightpos is only the arbitrary point on that ray the
// light's view matrix was placed at, a few tens of units above the world origin. A
// fragment far from the origin that treats that point as a real light source gets a
// direction tens of degrees away from the one the depth in the map was rasterised
// along, and every quantity derived from it - the normal offset below, the slope bias
// under it - is then computed for a light that is not the one casting.
//
// The transform itself knows the answer. lightmatrix maps world position to the clip
// space the shadow pass wrote, so its third row is the gradient of stored depth with
// respect to world position: the direction depth increases in, which is the direction
// the light shines. Reading it here costs nothing, needs no new uniform, and stays
// correct automatically when the volume is refitted to the camera every frame.
//
// A projective light matrix - a spot, which nothing builds today but setLightProjection
// accepts - has no single such direction, and is detected by its non-zero w row and
// sent back to the point-light form it genuinely wants.
vec3 shadowLightDirection(vec3 worldpos) {
    vec3 wrow = vec3(scene.lightmatrix[0].w, scene.lightmatrix[1].w, scene.lightmatrix[2].w);
    if (dot(wrow, wrow) < 1e-12) {
        return normalize(-vec3(scene.lightmatrix[0].z, scene.lightmatrix[1].z, scene.lightmatrix[2].z));
    }
    return normalize(scene.shadowlightpos.xyz - worldpos);
}
// How much of this fragment is in shadow: 0 fully lit, 1 fully shadowed. The bias is
// the one from color_frag.glsl, unchanged, so a surface starts self-shadowing at the
// same angle on both backends; what changed is the filtering.
//
// The old kernel walked a 3x3 or 5x5 grid of whole texels with equal weight. That is
// a box blur of a hard edge: it widens the penumbra but keeps its staircase, and the
// 5x5 form spends twenty-five taps doing it. This takes four bilinear taps on a ring
// instead - sixteen texels, each weighted - and rotates the ring by a per-pixel angle
// so that whatever aliasing is left lands as fine noise rather than as concentric
// bands, which the eye reads as a soft edge instead of an artefact.
float calculateShadow(vec3 normal) {
    vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowmap, 0));

    // Normal offset. A shadow map stores one depth per texel, so a surface lying at a
    // shallow angle to the light spans a whole texel with a range of depths while the
    // map holds a single value for all of it - and half of the surface then finds
    // itself behind its own recorded depth. On a large, nearly flat receiver like
    // terrain this does not look like the striping the effect is named for; it looks
    // like an entire region of the map going dark for no reason, because the filter
    // smooths the stripes into one mass.
    //
    // A depth bias cannot fix that without also lifting real shadows off their
    // objects. Moving the *sample point* along the surface normal can: the comparison
    // then happens on the same surface, a fraction of a texel away, where the stored
    // depth is the one this surface actually wrote.
    //
    // How far to move is decided by the sine of the angle between the surface and the
    // light, and getting that wrong is what put false shadow on every sunlit railing
    // in the scene. The step used to be sized so that the sample landed a fixed number
    // of texels sideways in the map, which forces the world-space step to grow as one
    // over that sine: unbounded for a surface pointing straight at the sun, and a
    // third of a world unit for one the sun merely grazes. A wall or a field of
    // terrain never notices, because a step that long still lands on the same surface.
    // The thin geometry a military base is made of - a handrail, a lattice brace, a
    // soldier's arm - is narrower than the step, so the comparison was walked clean
    // off the object and onto whatever else the light saw at that texel, and the lit
    // side of every beam ended up tested against a depth that was never its own.
    //
    // Scaling by the sine instead is what the geometry argues for. The depth a surface
    // spans inside one texel is that texel's width times the sine, so that is the
    // amount which has to be stepped over; a face pointing at the sun spans none of it
    // and needs no step at all, which is also the only value that stays finite there.
    // The sideways part of the step is then the texel width times the sine squared,
    // which never leaves the texel it started in - so the offset can no longer cross a
    // silhouette, however thin the caster is.
    float ndotl = clamp(dot(normal, shadowLightDirection(vWorldPos)), 0.0, 1.0);
    float sintheta = sqrt(1.0 - ndotl * ndotl);
    // Still the tangent of the same angle, and still clamped, so a surface seen almost
    // edge-on asks for a bounded number of texels rather than an unbounded one.
    float slope = clamp(sintheta / max(ndotl, 0.05), 0.0, 6.0);

    // What one texel of the map is worth out in the world, and what one world unit
    // along the light is worth in stored depth. Both are read straight out of the
    // light matrix: its first row is how many clip units a world unit spans across the
    // map, its third is how many it spans into it. Deriving them rather than passing
    // them in lets every number below be written in world units - the only units in
    // which "one texel" or "a centimetre of clearance" mean anything - and keeps them
    // right if the light's volume is ever resized or fitted to the camera, which no
    // constant tuned against one fixed box could survive.
    vec3 lightrowx = vec3(scene.lightmatrix[0].x, scene.lightmatrix[1].x, scene.lightmatrix[2].x);
    vec3 lightrowz = vec3(scene.lightmatrix[0].z, scene.lightmatrix[1].z, scene.lightmatrix[2].z);
    float texelworld = (2.0 * texelSize.x) / max(length(lightrowx), 1e-6);
    float depthperworld = length(lightrowz);

    // One texel of clearance, plus more as the light flattens against the surface, and
    // all of it faded out by the sine above as the surface turns to face the sun.
    float offsetworld = texelworld * (1.0 + 2.0 * slope) * sintheta;
    projCoords += (mat3(scene.lightmatrix) * normal) * offsetworld;

    // Only x and y are a texture coordinate and need the -1..1 to 0..1 mapping; z is
    // already the depth the shadow pass wrote.
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    // The map has a hard edge and the world does not. A lookup landing outside it
    // reads the sampler's white border and comes back lit, so the last shadow before
    // the boundary used to end on a straight line ruled across the ground. Fading the
    // term out over the outermost few per cent of the map trades a little shadow at
    // the rim for an edge nobody can pick out.
    vec2 fromcentre = abs(projCoords.xy * 2.0 - 1.0);
    float edgefade = clamp((1.0 - max(fromcentre.x, fromcentre.y)) * 12.0, 0.0, 1.0);
    if (edgefade <= 0.0) return 0.0;

    // What is left for a constant to do, once the normal offset has dealt with the
    // texel grid and the shadow pass's own slope-scaled depth bias has dealt with the
    // rasterised slope: three quarters of a texel of clearance head on, more as the
    // light flattens out, turned into stored depth by the light matrix. Writing it in
    // world units and converting is the whole point. The number that used to sit here
    // was in normalised depth, so what it forgave on the ground moved with the depth
    // of the light's frustum - deep enough to swallow a vehicle's shadow whole in one
    // scene, not deep enough to stop acne in the next.
    float currentDepth = projCoords.z;
    float bias = depthperworld * texelworld * (0.75 + 1.5 * slope);
    float compare = currentDepth - bias;
    // Interleaved gradient noise: cheap, and it decorrelates neighbouring pixels
    // without a lookup table.
    float noise = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    float angle = noise * 6.2831853;
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);
    const vec2 ring[4] = vec2[4](vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(-1.0, 0.0), vec2(0.0, -1.0));

    // One cheap pass that does two jobs at once.
    //
    // The first is classification. Most of the screen is either well inside the light
    // or well inside a shadow, and for those fragments no amount of filtering can
    // change the answer - filtering only matters along the edge, which is a thin band
    // of pixels. Four plain taps at the widest radius the filter below could reach
    // decide which case this is: none of them occluded means the whole footprint is
    // lit, all four occluded means it is entirely shadowed, and either way the
    // expensive part is skipped outright.
    //
    // The second is the blocker search behind contact hardening. A shadow in the
    // world is not equally soft everywhere: where an object touches the ground its
    // shadow is sharp, and it blurs as the gap grows, because the further away the
    // blocker is the larger the light appears from the receiver. Measuring that needs
    // the average depth of whatever stands between this fragment and the light -
    // which is exactly what these four taps have already read, so the search costs
    // nothing extra on top of the classification.
    float probespread = scene.softshadows != 0 ? 6.0 : 2.0;
    float blockersum = 0.0;
    float occluders = 0.0;
    for (int i = 0; i < 4; ++i) {
        vec2 offset = rotation * ring[i] * probespread * texelSize;
        float depth = texture(shadowmap, projCoords.xy + offset).r;
        if (depth < compare) {
            blockersum += depth;
            occluders += 1.0;
        }
    }
    if (occluders < 0.5) return 0.0;
    if (occluders > 3.5) return edgefade;

    float blockerdepth = blockersum / occluders;
    // Clamped so a blocker far above the receiver - a tower against the ground -
    // does not smear its shadow across the whole map, and so that a contact shadow
    // still gets enough width to be filtered rather than aliased.
    float penumbra = clamp((compare - blockerdepth) / max(blockerdepth, 1e-4) * 24.0, 0.6, 6.0);
    float spread = scene.softshadows != 0 ? penumbra : min(penumbra, 2.0);

    float lit = 0.0;
    for (int i = 0; i < 4; ++i) {
        lit += shadowTap(projCoords.xy + rotation * ring[i] * spread * texelSize, compare, texelSize);
    }
    return (1.0 - lit * 0.25) * edgefade;
}

void main() {
    // Defaults match pbr_frag.glsl exactly: mid grey albedo, dielectric, half rough,
    // fully unoccluded. The albedo map is raised to 2.2 because it is authored in
    // sRGB while the maths below is linear.
    vec3 albedo = pc.maps0.x > 0 ? pow(texture(albedoMap, vTexCoords).rgb, vec3(2.2)) : vec3(0.5);
    float metallic = pc.maps0.z > 0 ? texture(metallicMap, vTexCoords).r : 0.0;
    float roughness = pc.maps0.w > 0 ? texture(roughnessMap, vTexCoords).r : 0.5;
    float ao = pc.maps1.x > 0 ? texture(aoMap, vTexCoords).r : 1.0;

    vec3 N = pc.maps0.y > 0 ? getNormalFromMap() : normalize(vNormal);
    vec3 V = normalize(scene.viewpos.xyz - vWorldPos);

    // Computed once and applied to every light's contribution, matching mesh3d.frag:
    // the engine keeps a single shadow map for one casting light, so a second light
    // gets no shadow term of its own. The geometric normal goes in even when a normal
    // map is bound, because the bias is about how the real surface faces the light
    // rather than about its bumps. 1.0 means fully lit.
    float shadowing = 1.0;
    if (scene.shadowlightpos.w > 0.0) {
        shadowing = 1.0 - calculateShadow(normalize(vNormal));
    }

    // Reflectance at normal incidence: 0.04 for dielectrics, the albedo itself for
    // metals, blended by the metallic value.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    vec3 ambientSum = vec3(0.0);
    for (int i = 0; i < scene.lightnum; ++i) {
        if ((scene.enabledlights & (1 << i)) == 0) continue;

        int lightType = scene.lights[i].type;

        if (lightType == 0) {
            ambientSum += scene.lights[i].ambient.rgb;
            continue;
        }

        vec3 L;
        vec3 radiance;

        if (lightType == 1) {
            L = normalize(-scene.lights[i].direction);
            radiance = scene.lights[i].diffuse.rgb * PI;
        } else {
            L = normalize(scene.lights[i].position - vWorldPos);
            float distance = length(scene.lights[i].position - vWorldPos);
            float attenuation = 1.0 / (scene.lights[i].constant +
                    scene.lights[i].linear * distance +
                    scene.lights[i].quadratic * (distance * distance));
            radiance = scene.lights[i].diffuse.rgb * PI * attenuation;

            if (lightType == 3) {
                float theta = dot(L, normalize(-scene.lights[i].direction));
                float epsilon = scene.lights[i].cutOff - scene.lights[i].outerCutOff;
                float intensity = clamp((theta - scene.lights[i].outerCutOff) / epsilon, 0.0, 1.0);
                radiance *= intensity;
            }
        }

        vec3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        // Only the direct term is shadowed. The ambient one is accumulated separately
        // below and stays untouched, which is what keeps a shadowed surface from
        // going pure black - the same rule mesh3d.frag follows.
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadowing;
    }

    // The global ambient only stands in when the scene has no lights at all, which
    // is the same rule the non-PBR shader follows.
    vec3 ambientLight = ambientSum;
    if (dot(ambientLight, ambientLight) < 0.001 && scene.lightnum == 0) {
        ambientLight = scene.globalambientcolor.rgb;
    }
    vec3 ambient = ambientLight * albedo * ao;

    vec3 color = ambient + Lo;

    // Reinhard tonemap and gamma, applied unconditionally here - unlike the non-PBR
    // shader, where both hang off scene flags.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
