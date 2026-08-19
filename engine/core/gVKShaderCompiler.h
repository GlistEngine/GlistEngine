/*
 * gVKShaderCompiler.h
 *
 * Compiles the Vulkan 2D shaders from their GLSL sources at runtime, so a shader
 * can be edited and seen without rebuilding the engine. This is a development
 * aid: engine/CMakeLists.txt only defines GVK_RUNTIME_SHADERS for Debug builds
 * on machines that have libshaderc, and only then is the source directory of
 * graphics/shaders/vk baked in as GVK_VK_SHADER_SOURCE_DIR.
 *
 * When the feature is off - every Release build, and any machine without
 * shaderc - these functions report that nothing is available and the pipelines
 * fall back to the SPIR-V committed in gVKShaders.h. That fallback is the
 * shipping path; runtime compilation never changes what a release renders,
 * because both come from the same sources through the same compiler settings.
 */

#pragma once

#ifndef CORE_GVKSHADERCOMPILER_H
#define CORE_GVKSHADERCOMPILER_H

#include "gVKContext.h"

#ifdef GVK_VULKAN

#include <cstdint>
#include <string>
#include <vector>

// True when shaders can be compiled from source in this build.
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
bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		std::vector<uint32_t>& spirv);

#endif /* GVK_VULKAN */

#endif /* CORE_GVKSHADERCOMPILER_H */
