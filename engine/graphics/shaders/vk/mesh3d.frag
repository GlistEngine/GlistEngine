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
layout(location = 4) in mat3 vTBN;
layout(location = 7) in vec3 vTangentViewPos;
layout(location = 8) in vec3 vTangentFragPos;
layout(location = 9) in vec4 vFragPosLightSpace;

layout(location = 0) out vec4 outColor;

// One texture per set, each with a single binding, the same shape image2d.frag
// uses. That is what lets a gTexture's descriptor set - allocated once when the
// texture is uploaded - be bound here without a second set per material. A mesh
// with no map of a given kind gets a 1x1 white texture, so the binding is always
// valid even when the flags below say not to sample it.
layout(set = 1, binding = 0) uniform sampler2D diffusemap;
layout(set = 2, binding = 0) uniform sampler2D specularmap;
layout(set = 3, binding = 0) uniform sampler2D normalmap;
// The depth the scene was recorded at from the light's point of view. Like the
// material maps this binding is always filled - with the 1x1 white texture when no
// shadow map exists - and scene.shadowlightpos.w says whether to believe it.
layout(set = 4, binding = 0) uniform sampler2D shadowmap;

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
    Light lights[8];
} scene;

layout(push_constant) uniform Push {
    mat4 model;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    // x shininess, y useDiffuseMap, z useSpecularMap, w useNormalMap.
    vec4 misc;
} pc;

// How much of this fragment is in shadow: 0 fully lit, 1 fully shadowed. Ported
// from color_frag.glsl's calculateShadow, including the slope-scaled bias and the
// PCF kernel, so a surface lands in shadow at the same angle on both backends.
//
// One difference from OpenGL, and it is not a choice: there the light-space depth
// runs -1..1 and has to be rescaled with "* 0.5 + 0.5", while Vulkan's clip space
// already ends at 0..1. Rescaling here as well would push every comparison into the
// near half of the range and shadow the whole scene.
float calculateShadow(vec3 normal) {
    vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w;
    // Only x and y are a texture coordinate and need the -1..1 to 0..1 mapping; z is
    // already the depth the shadow pass wrote.
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(scene.shadowlightpos.xyz - vFragPos);
    // A surface seen edge-on by the light needs a larger bias, otherwise its own
    // depth quantisation makes it shadow itself in stripes.
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowmap, 0));
    float shadow = 0.0;
    float spacing = scene.softshadows != 0 ? 2.0 : 1.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowmap,
                    projCoords.xy + vec2(x, y) * texelSize * spacing).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

// The material colours the lighting actually uses, picked once in main(). Passed
// into the light functions rather than read from the push constant inside them,
// because a map replaces them and color_frag.glsl resolves that the same way.
//
// shadowing is 1 where the fragment is fully lit and 0 where it is fully shadowed,
// and it scales the diffuse and specular terms but never the ambient one - a
// shadowed surface still catches ambient light, which is what keeps it from going
// pure black.
vec4 calcAmbLight(Light light, vec4 matAmbient) {
    return light.ambient * matAmbient;
}

vec4 calcDirLight(Light light, vec3 normal, vec3 viewDir,
        vec4 matAmbient, vec4 matDiffuse, vec4 matSpecular, float shadowing) {
    // Not transformed into tangent space, matching color_frag.glsl: a directional
    // light keeps its world-space direction there even when normal mapping is on.
    vec3 lightDir = normalize(-light.direction);
    float diff;
    float spec;
    if (pc.misc.w > 0.0) {
        // Blinn-Phong while normal mapped, Phong otherwise - the OpenGL shader
        // switches models on the same flag, and the two do not agree, so following
        // it is what keeps the highlights identical.
        diff = max(dot(lightDir, normal), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), pc.misc.x);
    } else {
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), pc.misc.x);
    }

    vec4 ambient = light.ambient * matAmbient;
    vec4 diffuse = light.diffuse * vec4(diff) * matDiffuse * shadowing;
    vec4 specular = light.specular * vec4(spec) * matSpecular * shadowing;
    return ambient + diffuse + specular;
}

vec4 calcPointLight(Light light, vec3 normal, vec3 viewDir,
        vec4 matAmbient, vec4 matDiffuse, vec4 matSpecular, float shadowing) {
    vec3 lightDir;
    float distance;
    float diff;
    float spec;
    if (pc.misc.w > 0.0) {
        // A positional light does get moved into tangent space, so the direction and
        // the distance are both measured there.
        vec3 tangentLightPos = vTBN * light.position;
        lightDir = normalize(tangentLightPos - vTangentFragPos);
        distance = length(tangentLightPos - vTangentFragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), pc.misc.x);
    } else {
        lightDir = normalize(light.position - vFragPos);
        distance = length(light.position - vFragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), pc.misc.x);
    }

    vec4 ambient = light.ambient * matAmbient;
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
    float diff;
    float spec;
    if (pc.misc.w > 0.0) {
        // A positional light does get moved into tangent space, so the direction and
        // the distance are both measured there.
        vec3 tangentLightPos = vTBN * light.position;
        lightDir = normalize(tangentLightPos - vTangentFragPos);
        distance = length(tangentLightPos - vTangentFragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), pc.misc.x);
    } else {
        lightDir = normalize(light.position - vFragPos);
        distance = length(light.position - vFragPos);
        diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), pc.misc.x);
    }

    vec4 ambient = light.ambient * matAmbient;
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
        norm = normalize(texture(normalmap, vTexCoords).rgb * 2.0 - 1.0);
        viewDir = normalize(vTangentViewPos - vTangentFragPos);
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
        // over what is behind them.
        if (sampled.a < 0.5) discard;
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
}
