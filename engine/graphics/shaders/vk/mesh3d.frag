#version 450

// Lighting for the Vulkan 3D path, ported from color_frag.glsl so that a surface
// lit by the same gLight comes out the same on both backends. Diffuse, specular and
// normal maps are all here; shadow mapping and fog are not, and where
// color_frag.glsl branches on those this file takes the branch that is true without
// them.

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vColor;
layout(location = 3) in vec2 vTexCoords;
// The tangent, not the finished basis; see mesh3d.vert for why the TBN matrix and
// the two tangent-space positions stopped being varyings.
layout(location = 4) in vec3 vTangent;
layout(location = 5) in vec4 vFragPosLightSpace;

// World to tangent space, and the two positions expressed in it. Rebuilt in main()
// for a normal-mapped material and left at their identity values otherwise, which is
// exactly when the light helpers below read them: they are globals rather than
// parameters so that those helpers keep the signature they have on the OpenGL side.
mat3 tbn = mat3(1.0);
vec3 tangentviewpos = vec3(0.0);
vec3 tangentfragpos = vec3(0.0);

layout(location = 0) out vec4 outColor;

// The three material maps share one set, the way mesh3dpbr.frag's five do, and for
// the same reason: Vulkan guarantees only four bound descriptor sets, and Adreno
// and Mali offer exactly four. A set per texture would put this pipeline at five
// with the scene block and the shadow map, so its layout would simply fail to be
// created on most Android phones and the whole Vulkan backend would refuse to
// start. gVKRenderEngine allocates one of these per distinct combination of maps
// and caches it. A mesh with no map of a given kind gets a 1x1 white texture, so
// every binding is valid even when the flags below say not to sample it.
layout(set = 1, binding = 0) uniform sampler2D diffusemap;
layout(set = 1, binding = 1) uniform sampler2D specularmap;
layout(set = 1, binding = 2) uniform sampler2D normalmap;
// The depth the scene was recorded at from the light's point of view. Like the
// material maps this binding is always filled - with the 1x1 white texture when no
// shadow map exists - and scene.shadowlightpos.w says whether to believe it.
layout(set = 2, binding = 0) uniform sampler2D shadowmap;

// Whether this pipeline's fragments may discard. A tile based mobile GPU decides
// whether it can reject a fragment on depth *before* running this shader, and it
// gives that up for any shader that contains a discard at all - the depth a
// discarded fragment would have written is not known until the shader has run, so
// Adreno's low resolution Z and Mali's forward pixel kill both switch off. One
// static discard in here therefore costs early rejection on every mesh in the
// scene, not only the ones that punch holes.
//
// A specialization constant is resolved when the pipeline is compiled, so setting
// this to zero removes the branch and the discard with it, and the driver sees a
// shader that always writes the depth it was given. gVKPipeline builds both forms
// and gVKRenderEngine picks between them per draw: a material whose diffuse map
// was found to contain no alpha below the cutout threshold - which is most of
// them - goes through the copy that keeps early rejection.
layout(constant_id = 0) const int GVK_CUTOUT = 1;

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
    // Same bits as color_frag.glsl; see gRenderer::ENABLE_FOG and friends.
    int flags;
    // xyz fog colour, w mode: 0 linear, 1 exponential.
    vec4 fogcolor;
    // x density, y gradient, z linear start, w linear end.
    vec4 fogparams;
    Light lights[8];
} scene;

const int ENABLE_FOG_FLAG = 1 << 1;
const int ENABLE_GAMMA_FLAG = 1 << 2;
const int ENABLE_HDR_FLAG = 1 << 3;

// The same curve color_frag.glsl uses, so a scene fogged on one backend is fogged
// the same amount on the other.
float fogVisibility(float distance) {
    if (scene.fogcolor.w < 0.5) {
        float len = scene.fogparams.w - scene.fogparams.z;
        return clamp((scene.fogparams.w - distance) / len, 0.0, 1.0);
    }
    return clamp(exp(-pow(distance * scene.fogparams.x, scene.fogparams.y)), 0.0, 1.0);
}

layout(push_constant) uniform Push {
    mat4 model;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    // x shininess, y useDiffuseMap, z useSpecularMap, w useNormalMap.
    vec4 misc;
} pc;

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
    float ndotl = clamp(dot(normal, shadowLightDirection(vFragPos)), 0.0, 1.0);
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

// The material colours the lighting actually uses, picked once in main(). Passed
// into the light functions rather than read from the push constant inside them,
// because a map replaces them and color_frag.glsl resolves that the same way.
//
// shadowing is 1 where the fragment is fully lit and 0 where it is fully shadowed,
// and it scales the diffuse and specular terms but never the ambient one - a
// shadowed surface still catches ambient light, which is what keeps it from going
// pure black.
// How much ambient light reaches a surface with this world-space normal. Ambient in
// this shader used to be a single number added to every surface at every angle,
// which is the main reason a scene lit this way looks flat: it is the one term that
// carries no shape at all, and at the levels applications actually use - half grey
// is common - it washes out the shading that the directional terms provide.
//
// Ambient light in the world arrives mostly from the sky, bounces off the ground,
// and reaches a downward-facing surface far less than an upward-facing one. This is
// that, in its cheapest honest form: a hemisphere, weighted by how far the normal
// leans towards the sky. The two weights average to exactly one, so the scene keeps
// the ambient level and the ambient colour the application asked for - no tint is
// introduced and nothing gets brighter overall. What changes is that a wall, a roof
// and the ground stop receiving the same fill light.
//
// Always the geometric normal, never the mapped one: normal mapping works in
// tangent space, where "up" has no meaning.
float ambientOcclusionByNormal(vec3 worldnormal) {
    return mix(0.75, 1.25, worldnormal.y * 0.5 + 0.5);
}

vec4 calcAmbLight(Light light, vec4 matAmbient) {
    return light.ambient * matAmbient * ambientOcclusionByNormal(normalize(vNormal));
}

// The specular lobe, for every light type and both the normal-mapped and the plain
// path, replacing the Blinn-Phong and Phong pair that used to be picked between.
//
// Those two describe a highlight as "raise a cosine to a power", which has no
// physical footing: the lobe does not narrow the way a real microfacet surface's
// does, it carries no Fresnel so a surface never brightens as it turns edge-on to
// the viewer - the effect that makes wet roads and painted metal read as what they
// are - and its brightness drifts with the exponent instead of conserving energy.
//
// This is the Cook-Torrance form used by the PBR path in mesh3dpbr.frag: a GGX
// distribution for how the microfacets are oriented, Smith's term for how they
// shadow each other, and Schlick's Fresnel for how reflective the surface is at this
// angle. The material's shininess is read as the roughness it was always standing in
// for, so a material authored for the old model keeps its character - a high
// shininess is still a tight highlight - and the material's own specular colour
// still scales the result, so how reflective a surface is remains the artist's call.
float specularLobe(vec3 normal, vec3 viewDir, vec3 lightDir) {
    float roughness = clamp(sqrt(2.0 / (max(pc.misc.x, 1.0) + 2.0)), 0.04, 1.0);
    float a = roughness * roughness;
    float a2 = a * a;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 1e-4);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float VdotH = max(dot(viewDir, halfwayDir), 0.0);

    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    float D = a2 / max(3.14159265 * d * d, 1e-6);

    float k = a * 0.5;
    float G = (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));

    // The dielectric baseline, 4% reflective head on, rising to fully reflective at
    // a grazing angle. A material that is meant to look like metal says so through
    // its specular colour, which multiplies this outside the function.
    float F = 0.04 + 0.96 * pow(1.0 - VdotH, 5.0);

    // The cosine that belongs to the specular term is folded in here, because unlike
    // the diffuse one it is not applied by the caller.
    return D * G * F * NdotL / (4.0 * NdotV);
}

vec4 calcDirLight(Light light, vec3 normal, vec3 viewDir,
        vec4 matAmbient, vec4 matDiffuse, vec4 matSpecular, float shadowing) {
    // Not transformed into tangent space, matching color_frag.glsl: a directional
    // light keeps its world-space direction there even when normal mapping is on.
    vec3 lightDir = normalize(-light.direction);
    // One lobe for both paths now. The old code picked Blinn-Phong while normal
    // mapped and Phong otherwise, because the OpenGL shader does; neither is a
    // microfacet model, and the two disagreeing with each other meant the same
    // material changed character the moment a normal map was attached to it.
    float diff = max(dot(lightDir, normal), 0.0);
    float spec = specularLobe(normal, viewDir, lightDir);

    vec4 ambient = light.ambient * matAmbient * ambientOcclusionByNormal(normalize(vNormal));
    vec4 diffuse = light.diffuse * vec4(diff) * matDiffuse * shadowing;
    vec4 specular = light.specular * vec4(spec) * matSpecular * shadowing;
    return ambient + diffuse + specular;
}

vec4 calcPointLight(Light light, vec3 normal, vec3 viewDir,
        vec4 matAmbient, vec4 matDiffuse, vec4 matSpecular, float shadowing) {
    vec3 lightDir;
    float distance;
    if (pc.misc.w > 0.0) {
        // A positional light does get moved into tangent space, so the direction and
        // the distance are both measured there.
        vec3 tangentLightPos = tbn * light.position;
        lightDir = normalize(tangentLightPos - tangentfragpos);
        distance = length(tangentLightPos - tangentfragpos);
    } else {
        lightDir = normalize(light.position - vFragPos);
        distance = length(light.position - vFragPos);
    }
    // Which space the direction was measured in is the only thing that differed
    // between these two branches; the lobe itself is the same one either way.
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = specularLobe(normal, viewDir, lightDir);

    vec4 ambient = light.ambient * matAmbient * ambientOcclusionByNormal(normalize(vNormal));
    vec4 diffuse = light.diffuse * diff * matDiffuse * shadowing;
    vec4 specular = light.specular * spec * matSpecular * shadowing;

    float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));
    return (ambient + diffuse + specular) * attenuation;
}

vec4 calcSpotLight(Light light, vec3 normal, vec3 viewDir,
        vec4 matAmbient, vec4 matDiffuse, vec4 matSpecular, float shadowing) {
    vec3 lightDir;
    float distance;
    if (pc.misc.w > 0.0) {
        // A positional light does get moved into tangent space, so the direction and
        // the distance are both measured there.
        vec3 tangentLightPos = tbn * light.position;
        lightDir = normalize(tangentLightPos - tangentfragpos);
        distance = length(tangentLightPos - tangentfragpos);
    } else {
        lightDir = normalize(light.position - vFragPos);
        distance = length(light.position - vFragPos);
    }
    // Which space the direction was measured in is the only thing that differed
    // between these two branches; the lobe itself is the same one either way.
    float diff = max(dot(normal, lightDir), 0.0);
    float spec = specularLobe(normal, viewDir, lightDir);

    vec4 ambient = light.ambient * matAmbient * ambientOcclusionByNormal(normalize(vNormal));
    vec4 diffuse = light.diffuse * diff * matDiffuse * shadowing;
    vec4 specular = light.specular * spec * matSpecular * shadowing;

    float attenuation = 1.0 / (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));

    // The cone falls off between the inner and the outer angle; outside it the
    // intensity clamps to 0 and only the ambient term survives.
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return ambient + diffuse + specular;
}

void main() {
    // With a normal map the whole lighting calculation moves into tangent space:
    // the normal comes from the texture (unpacked from 0..1 to -1..1) and the view
    // direction is the tangent-space one the vertex stage produced.
    vec3 norm;
    vec3 viewDir;
    if (pc.misc.w > 0.0) {
        // Gram-Schmidt, moved here from the vertex stage: the tangent is
        // re-orthogonalised against the normal - interpolation leaves neither
        // normalised nor perpendicular - and the bitangent is their cross product
        // rather than a vertex attribute. Transposing an orthonormal basis inverts
        // it, so tbn maps world space into tangent space, which is the direction
        // everything below needs. A material with no normal map never gets here.
        vec3 N = normalize(vNormal);
        vec3 T = normalize(vTangent);
        T = normalize(T - dot(T, N) * N);
        tbn = transpose(mat3(T, cross(N, T), N));
        tangentviewpos = tbn * scene.viewpos.xyz;
        tangentfragpos = tbn * vFragPos;

        norm = normalize(texture(normalmap, vTexCoords).rgb * 2.0 - 1.0);
        viewDir = normalize(tangentviewpos - tangentfragpos);
    } else {
        norm = normalize(vNormal);
        viewDir = normalize(scene.viewpos.xyz - vFragPos);
    }

    // Computed once and handed to every light, matching color_frag.glsl - the
    // engine keeps a single shadow map for one casting light, so a second light
    // does not get a shadow term of its own. The geometric normal is used even
    // when normal mapping is on, because the bias is about the surface's real
    // orientation towards the light rather than its bumps. 1.0 means fully lit,
    // which is also what an unshadowed scene gets.
    float shadowing = 1.0;
    if (scene.shadowlightpos.w > 0.0) {
        shadowing = 1.0 - calculateShadow(normalize(vNormal));
    }

    // A diffuse map replaces both the ambient and the diffuse colour, exactly as in
    // color_frag.glsl - the map is treated as the surface's colour, not as a tint on
    // top of the material.
    //
    // On top of that, the push colours arrive with renderColor already folded in;
    // see drawMesh3D for why that has to happen per draw rather than through the
    // scene block. Where the colour comes from a texture the push slot carries
    // nothing but renderColor and scales the sample, and where it does not the slot
    // is the material colour times renderColor and is used as it is. Either way
    // what reaches the lighting below is the surface colour times renderColor.
    vec4 matAmbient;
    vec4 matDiffuse;
    if (pc.misc.y > 0.0) {
        vec4 sampled = texture(diffusemap, vTexCoords);
        matAmbient = sampled * pc.ambient;
        matDiffuse = sampled * pc.diffuse;
        // Cutout transparency: the OpenGL path discards these fragments rather than
        // blending them, which is what keeps foliage and fences from writing depth
        // over what is behind them. Compiled out entirely where the material cannot
        // produce such a texel; see GVK_CUTOUT above.
        if (GVK_CUTOUT != 0 && sampled.a < 0.5) discard;
    } else {
        matAmbient = pc.ambient;
        matDiffuse = pc.diffuse;
    }

    vec4 matSpecular = pc.misc.z > 0.0 ? texture(specularmap, vTexCoords) * pc.specular : pc.specular;

    vec4 result = vec4(0.0);
    bool haslight = false;
    for (int i = 0; i < scene.lightnum; i++) {
        if ((scene.enabledlights & (1 << i)) == 0) continue;

        if (scene.lights[i].type == 0) {
            result += calcAmbLight(scene.lights[i], matAmbient);
        } else if (scene.lights[i].type == 1) {
            result += calcDirLight(scene.lights[i], norm, viewDir, matAmbient, matDiffuse, matSpecular, shadowing);
        } else if (scene.lights[i].type == 2) {
            result += calcPointLight(scene.lights[i], norm, viewDir, matAmbient, matDiffuse, matSpecular, shadowing);
        } else if (scene.lights[i].type == 3) {
            result += calcSpotLight(scene.lights[i], norm, viewDir, matAmbient, matDiffuse, matSpecular, shadowing);
        } else {
            return;
        }
        haslight = true;
    }
    // An unlit scene is not black: OpenGL falls back to the global ambient times the
    // material's *ambient* colour - not its diffuse - and this has to match, or the
    // two backends disagree on every scene that has no gLight in it.
    if (!haslight) {
        result = scene.globalambientcolor * matAmbient;
    }

    // color_frag.glsl ends with "result * renderColor * vec4(incolor, 1.0)", so a
    // mesh on the OpenGL path is tinted by whatever colour setColor was last given -
    // text included. renderColor is not applied here because it is already in the
    // three material colours above, folded in per draw; the scene block's copy is
    // one value for the whole frame and cannot follow a colour that changes between
    // two meshes.
    //
    // Only mesh3d, not mesh3dpbr.frag: pbr_frag.glsl does not apply it either, so
    // the two backends agree on PBR meshes by both leaving it alone.
    outColor = result * vec4(vColor, 1.0);

    // Fog, tone mapping and gamma, in the order color_frag.glsl applies them. The
    // distance is the view space depth rather than the radial distance to the eye,
    // which is what "abs(EyePosition.z / EyePosition.w)" comes to there; taking the
    // length of the eye vector instead would fog the edges of the screen more than
    // the centre and the two backends would part company at wide fields of view.
    if ((scene.flags & ENABLE_FOG_FLAG) > 0) {
        vec4 eyepos = scene.view * vec4(vFragPos, 1.0);
        float distance = abs(eyepos.z / eyepos.w);
        outColor = mix(vec4(scene.fogcolor.rgb, 1.0), outColor, fogVisibility(distance));
    }
    if ((scene.flags & ENABLE_HDR_FLAG) > 0) {
        outColor = vec4(outColor.rgb / (outColor.rgb + vec3(1.0)), outColor.a);
    }
    if ((scene.flags & ENABLE_GAMMA_FLAG) > 0) {
        outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
    }
}
