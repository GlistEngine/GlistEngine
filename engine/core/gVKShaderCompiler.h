/*
 * gVKShaderCompiler.h
 *
 * Compiles Vulkan GLSL to SPIR-V at runtime. In a development build with
 * shaderc, this also lets an engine shader be edited without rebuilding; only
 * that configuration bakes graphics/shaders into GVK_VK_SHADER_SOURCE_DIR.
 *
 * Desktop Release builds use glslang for application-provided source such as
 * post-process effects, while engine pipelines still use the SPIR-V committed
 * in gVKShaders.h. Without either compiler, file-backed user shaders can fall
 * back to adjacent .spv files; a source-only shader reports a clear failure.
 */

#pragma once

#ifndef CORE_GVKSHADERCOMPILER_H
#define CORE_GVKSHADERCOMPILER_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

#include <cstdint>
#include <string>
#include <vector>

// True when engine shader files can be hot reloaded in this build. Source-only
// application shaders may still be compiled by the Release glslang path when
// this reports false.
bool gvkRuntimeShadersAvailable();

// Directory the .vert / .frag sources are read from; empty when unavailable.
std::string gvkShaderSourceDir();

// Last modification time of one shader source, as a plain integer to compare
// against a previous reading. 0 when the file cannot be stated, which includes
// every build where runtime shaders are off.
long long gvkShaderFileTimestamp(const std::string& fileName);

// Compiles one shader source to SPIR-V. Returns false, with the compiler's
// diagnostics logged, when the file cannot be read or does not compile - the
// caller is expected to keep using whatever it had.
// define names one macro to set while compiling, or is empty for none. It exists
// because the shader sources are shared with the OpenGL backend and one of them -
// the coloured-geometry shader, which OpenGL draws both its 2D primitives and its
// 3D meshes with - is compiled twice under different markers. Kept identical to
// what gen_vk_shaders.cmake passes glslc, so a hot reloaded shader is the shader
// the release build would have used.
bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		const std::string& define,
		std::vector<uint32_t>& spirv);

// Compiles GLSL held in memory rather than read from the shader directory, for
// shaders an application loads through gShader. debugName only names the module
// in diagnostics.
//
// Unlike the engine's own shaders these keep their debug names: a user shader is
// addressed by name - setFloat("intensity", v) - and the names have to survive
// into the SPIR-V for reflection to find them. Optimisation is unaffected; the
// two are separate switches.
bool gvkCompileShaderSource(const std::string& source, VkShaderStageFlagBits stage,
		const std::string& debugName, std::vector<uint32_t>& spirv);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKSHADERCOMPILER_H */
