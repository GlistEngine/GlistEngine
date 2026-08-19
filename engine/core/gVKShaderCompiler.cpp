/*
 * gVKShaderCompiler.cpp
 */

#include "gVKShaderCompiler.h"

#ifdef GVK_VULKAN

#include "gUtils.h"

#ifdef GVK_RUNTIME_SHADERS
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#endif

bool gvkRuntimeShadersAvailable() {
#ifdef GVK_RUNTIME_SHADERS
	return !gvkShaderSourceDir().empty();
#else
	return false;
#endif
}

std::string gvkShaderSourceDir() {
#if defined(GVK_RUNTIME_SHADERS) && defined(GVK_VK_SHADER_SOURCE_DIR)
	return std::string(GVK_VK_SHADER_SOURCE_DIR) + "/";
#else
	return "";
#endif
}

long long gvkShaderFileTimestamp(const std::string& fileName) {
#ifdef GVK_RUNTIME_SHADERS
	const std::string path = gvkShaderSourceDir() + fileName;
	if(path.empty()) return 0;
	struct stat info;
	if(stat(path.c_str(), &info) != 0) return 0;
	return static_cast<long long>(info.st_mtime);
#else
	(void)fileName;
	return 0;
#endif
}

#ifdef GVK_RUNTIME_SHADERS

bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		std::vector<uint32_t>& spirv) {
	const std::string path = gvkShaderSourceDir() + fileName;
	std::ifstream file(path);
	if(!file.is_open()) {
		gLoge("gVKShaderCompiler") << "Could not open the shader source " << path;
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	const std::string source = buffer.str();

	shaderc_shader_kind kind;
	switch(stage) {
	case VK_SHADER_STAGE_VERTEX_BIT: kind = shaderc_vertex_shader; break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: kind = shaderc_fragment_shader; break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: kind = shaderc_geometry_shader; break;
	case VK_SHADER_STAGE_COMPUTE_BIT: kind = shaderc_compute_shader; break;
	default:
		gLoge("gVKShaderCompiler") << "Unsupported shader stage for " << fileName;
		return false;
	}

	// The same settings compile_vk_shaders.sh passes to glslc, so a shader behaves
	// identically whether it came from here or from the committed SPIR-V.
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, path.c_str(), options);
	if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
		gLoge("gVKShaderCompiler") << fileName << ": " << result.GetErrorMessage();
		return false;
	}
	spirv.assign(result.cbegin(), result.cend());
	return !spirv.empty();
}

#else

bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		std::vector<uint32_t>& spirv) {
	(void)fileName;
	(void)stage;
	(void)spirv;
	return false;
}

#endif /* GVK_RUNTIME_SHADERS */

#endif /* GVK_VULKAN */
