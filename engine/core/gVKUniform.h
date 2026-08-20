/*
 * gVKUniform.h
 *
 * The per-frame uniform buffer of the Vulkan 3D path: camera matrices and the
 * scene's lights, in the std140 layout the mesh3d shaders declare.
 *
 * There is one buffer per frame in flight rather than a single shared one. The CPU
 * writes next frame's values while the GPU may still be reading the previous
 * frame's, so a single buffer would be rewritten underneath work already submitted.
 * Each buffer stays permanently mapped: the data is rewritten every frame, and
 * mapping and unmapping around each write would cost more than the memory saved.
 *
 * Created by: Veysel Burak Eroglu.
 */

#pragma once

#ifndef CORE_GVKUNIFORM_H
#define CORE_GVKUNIFORM_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

#include <glm/glm.hpp>

// How many lights the shader's array holds. Mirrors GLIST_MAX_LIGHTS in
// gRenderer.h; the static_assert in gVKUniform.cpp keeps the two from drifting.
inline constexpr int GVK_MAX_LIGHTS = 8;

/*
 * One light, laid out to match the Light struct in mesh3d.frag under std140. The
 * alignas values are what produce that layout: a vec3 aligns to 16 bytes even
 * though it occupies 12, which is the rule that trips up hand-written UBO structs.
 *
 * Deliberately the same shape as gRenderer::gSceneLightData, since the OpenGL path
 * already feeds that exact layout to its own shader.
 */
struct alignas(16) gVKLightData {
	alignas(4) int type;
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 direction;
	alignas(16) glm::vec4 ambient;
	alignas(16) glm::vec4 diffuse;
	alignas(16) glm::vec4 specular;
	alignas(4) float constant;
	alignas(4) float linear;
	alignas(4) float quadratic;
	alignas(4) float cutoff;
	alignas(4) float outercutoff;
};

/*
 * The whole Scene uniform block, matching set 0 binding 0 of the mesh3d shaders.
 * pad0/pad1 are not decoration: std140 starts the light array on a 16 byte
 * boundary, so the two ints before it have to be followed by eight bytes of
 * nothing, and naming them keeps that visible instead of accidental.
 */
struct alignas(16) gVKSceneUniforms {
	alignas(16) glm::mat4 projection;
	alignas(16) glm::mat4 view;
	// World space -> the light's clip space, for looking a fragment up in the shadow
	// map. Identity when there is no shadow map, in which case useshadowmap is 0 and
	// the shader never uses it.
	alignas(16) glm::mat4 lightmatrix;
	alignas(16) glm::vec4 viewpos;
	alignas(16) glm::vec4 globalambientcolor;
	// The renderer's current draw colour, kept for layout only: both 3D shaders
	// still declare this field, so the struct has to match, but neither reads it.
	//
	// It started here and had to move. color_frag.glsl ends with
	// "result * renderColor", and OpenGL republishes the value inside setColor, so a
	// canvas that recolours between two meshes gets a different colour on each. A
	// uniform block cannot follow that: it is read when the commands execute, not
	// when they are recorded, so every draw of the frame sees whichever value was
	// written last. drawMesh3D folds the colour into the material push constant
	// instead, which is per draw by construction.
	alignas(16) glm::vec4 rendercolor;
	// xyz is the shadow-casting light's position, which the depth bias is computed
	// from; w is 0 or 1 for whether a shadow map is bound at all.
	alignas(16) glm::vec4 shadowlightpos;
	alignas(4) int lightnum;
	alignas(4) int enabledlights;
	// 5x5 PCF instead of 3x3, mirroring ENABLE_SOFT_SHADOWS on the OpenGL side.
	alignas(4) int softshadows;
	// The same bitfield color_frag.glsl reads, with the same bit values - see
	// gRenderer::ENABLE_FOG and friends. Soft shadows keep their own field above,
	// which predates this and is what the shadow code already reads; the bit is set
	// here too so the two never disagree about what the renderer was asked for.
	// SSAO's bit is carried but nothing acts on it: it needs a depth and normal
	// prepass, which this backend does not have.
	alignas(4) int flags;
	// xyz the fog colour, w the mode (0 linear, 1 exponential). Packed into the
	// spare component rather than given an int of its own, which std140 would have
	// to pad out to sixteen bytes anyway.
	alignas(16) glm::vec4 fogcolor;
	// x density, y gradient, z linear start, w linear end.
	alignas(16) glm::vec4 fogparams;
	gVKLightData lights[GVK_MAX_LIGHTS];
};

/*
 * The push constant block of the mesh3d pipeline. Held to 128 bytes, the smallest
 * range Vulkan guarantees, so no implementation has to be asked for more.
 */
struct gVKMeshPush {
	glm::mat4 model;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	// x is shininess; the rest keeps the block vec4 aligned.
	glm::vec4 misc;
};

/*
 * The push constant block of the PBR pipeline, matching mesh3dpbr.*. Smaller than
 * the one above because a PBR material carries no colours - everything comes from
 * the maps, or from the shader's defaults where a map is absent.
 */
struct gVKPbrPush {
	glm::mat4 model;
	// maps0 = albedo, normal, metallic, roughness; maps1.x = ao. Non-zero means the
	// material supplies that map.
	glm::ivec4 maps0;
	glm::ivec4 maps1;
};

/*
 * The push constant block of the shadow pipeline, matching shadow3d.*. lightmodel is
 * lightProjection * lightView * model already multiplied out, so the depth pass needs
 * no matrices of its own.
 */
struct gVKShadowPush {
	glm::mat4 lightmodel;
	// x is non-zero when the caster has a cutout diffuse map; the rest keeps the
	// block vec4 aligned.
	glm::vec4 misc;
};

// A frame can change its lights while recording (muzzle flashes are a common
// example). Each change needs immutable storage until that frame's fence signals;
// otherwise rewriting one mapped UBO retroactively changes earlier draws.
/*
 * Creates one uniform buffer and one descriptor set per frame in flight. Must run
 * after the pipelines, because the set layout and the descriptor pool it allocates
 * from are both produced while reflecting the mesh3d shaders.
 */
bool gvkCreateUniformResources(gVKContext& ctx);
void gvkDestroyUniformResources(gVKContext& ctx);

/*
 * Adds one more chunk of scene uniform slots to a frame in flight. Not a file
 * local helper because gVKContext has to befriend it: the storage it appends to
 * is the context's own.
 */
bool gvkAppendSceneUniformChunk(gVKContext& ctx, int framei);

/*
 * Copies data into the current frame's buffer. Cheap: the buffer is host visible,
 * host coherent and already mapped, so this is a memcpy with no flush and no
 * synchronisation of its own - the frame's fence is what keeps it safe.
 */
bool gvkWriteSceneUniforms(gVKContext& ctx, const gVKSceneUniforms& data);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKUNIFORM_H */
