/*
 * gVKUserShader.h
 *
 * The Vulkan side of gShader: a shader an application wrote, loaded at run time
 * and addressed by name.
 *
 * The two backends disagree about what a shader is. OpenGL compiles GLSL, links
 * a program, and lets a caller reach any uniform in it by name at any time.
 * Vulkan has no free-standing uniforms and no linking step: a shader is SPIR-V,
 * its resources are descriptor sets and push constants, and the layout of both is
 * fixed when the pipeline is created - together with the vertex layout, the blend
 * state and the render pass. There is nothing to set a uniform on.
 *
 * What closes that gap is reflection. gVKReflect reads the compiled SPIR-V back
 * and reports every uniform block member by name, offset and shape, so
 * setVec3("lightpos", v) becomes a write at a known offset into a uniform buffer
 * the module owns, and setInt("tex", 0) becomes a note that binding N samples
 * whatever texture unit 0 holds. The application keeps the OpenGL-shaped API it
 * already uses; the difference stays here.
 *
 * What an application has to change: nothing in its C++, but its GLSL has to be
 * Vulkan GLSL. Loose uniforms are not a thing to translate into - they have to be
 * gathered into a block with a set and binding, and samplers need a binding of
 * their own. That is a shader source change, not an engine one, which is why the
 * failure is reported by name rather than papered over.
 *
 * Where the SPIR-V comes from:
 *   - shaderc, when the build has it (desktop development builds). GLSL goes in
 *     as the application wrote it and comes out compiled, hot reload included.
 *   - glslang, when a desktop release has its runtime libraries. This compiles
 *     source-only shaders once when the application constructs the effect.
 *   - a .spv file beside the source otherwise. This is the mobile path and the
 *     fallback for a desktop system without either compiler: the shader is
 *     compiled ahead of time by glslc and shipped next to the GLSL - the same
 *     arrangement the engine's own shaders use through gVKShaders.h.
 */

#pragma once

#ifndef CORE_GVKUSERSHADER_H
#define CORE_GVKUSERSHADER_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

#include "gVKReflect.h"
#include <string>
#include <vector>

struct gVKTexture;

// Handles are opaque and never reused within a run. 0 means "no shader".
using gVKUserShaderId = uint32_t;
inline constexpr gVKUserShaderId GVK_NO_USER_SHADER = 0;

// Shaders the engine builds through this path rather than as a pipeline of its
// own, and which therefore have to work in a build that links no shader compiler.
// There is one: gFbo's screen resolve, the pass that ends a post-process chain.
// Its SPIR-V is compiled at build time from graphics/shaders/fbo_vert.glsl and
// fbo_frag.glsl and lives in gVKShaders.h, and is used when compiling is not
// possible.
// Anything an application loads has no such fallback and passes NONE.
enum gvkBuiltinShader {
	GVK_BUILTIN_NONE = 0,
	GVK_BUILTIN_FBO,
};

// Compiles and builds one shader from GLSL held in memory. sourcePath, when the
// sources came from files, is where a pre-compiled .spv is looked for; leave it
// empty for sources the application built in memory. Returns GVK_NO_USER_SHADER
// on any failure, having logged why.
gVKUserShaderId gvkCreateUserShader(gVKContext& ctx, const std::string& vertexSource,
		const std::string& fragmentSource, const std::string& vertexPath,
		const std::string& fragmentPath, int builtin = GVK_BUILTIN_NONE);

// Releases one. The pipeline may still be recorded into a frame in flight, so the
// caller is responsible for having waited; gVKRenderEngine does that at teardown.
void gvkDestroyUserShader(gVKContext& ctx, gVKUserShaderId id);
void gvkDestroyAllUserShaders(gVKContext& ctx);

// Rebuilds every live shader's pipeline and descriptor sets. Called after a
// swapchain or render pass rebuild, which invalidates both.
bool gvkRebuildUserShaders(gVKContext& ctx);

// gShader::use(). The bound shader is what the next draw records with; binding
// GVK_NO_USER_SHADER puts the engine's own pipelines back.
void gvkUseUserShader(gVKUserShaderId id);
gVKUserShaderId gvkBoundUserShader();

// Writes one value by name. component, rows and columns describe what data
// points at - a glm::mat4 is FLOAT 4x4, a glm::vec3 is FLOAT 3x1 - and the write
// is converted into whatever layout the block declared, which is not the same
// thing: a mat3 is nine tight floats here and three sixteen-byte columns there.
//
// Returns false when the shader declares no such name, which is not an error in
// itself: the OpenGL path silently ignores a uniform the shader optimised away,
// and this keeps that behaviour rather than making every unused setter fatal.
bool gvkSetUserShaderValue(gVKUserShaderId id, const std::string& name, const void* data,
		gVKMemberComponent component, uint32_t rows, uint32_t columns);

// Points a sampler at a texture unit, which is what setInt("tex", 0) means on the
// OpenGL side.
bool gvkSetUserShaderTexture(gVKUserShaderId id, const std::string& name, int unit);

// True when the shader declares this name at all, block member or sampler.
bool gvkUserShaderHasUniform(gVKUserShaderId id, const std::string& name);

// How gVKUserShader reaches the texture bound to one unit without depending on
// the renderer. gVKRenderEngine installs this at start-up.
using gVKUserShaderTextureResolver = gVKTexture* (*)(int unit);
void gvkSetUserShaderTextureResolver(gVKUserShaderTextureResolver resolver);

// Records the bound shader - pipeline, descriptor sets and push constants - into
// the current command buffer, ready for a draw. Returns false when no user shader
// is bound or it could not be made ready, and the caller draws through the
// engine's own pipeline instead.
//
// blending and lines pick the pipeline variant, the same way the mesh path picks
// between the engine's own: blend state and topology class are baked into a
// pipeline and cannot be changed by a command.
bool gvkBindUserShaderForDraw(gVKContext& ctx, VkCommandBuffer cmd, bool blending, bool lines);

// Bytes per vertex of the layout the bound shader's pipeline describes, and
// whether that layout is the engine's mesh one. The fullscreen quad path checks
// both before feeding a shader vertices it would misread.
uint32_t gvkBoundUserShaderVertexStride();
bool gvkBoundUserShaderUsesMeshLayout();

#endif /* GVK_VULKAN */

#endif /* CORE_GVKUSERSHADER_H */
