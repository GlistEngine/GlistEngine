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

// Lifts the version out of a shared shader's Vulkan block and removes every
// #version line from what the compiler is given.
//
// The shader states its version in each dialect block, which is where it belongs
// and what gShader reads on the OpenGL side. glslang will not take it that way:
// GLSL requires #version to be the first thing in a shader and that is checked
// before any #if is resolved. So it is carried across as a forced version instead,
// exactly as gen_vk_shaders.cmake carries it into glslc's -std, and the file stays
// the one place the version is written down.
//
// Returns false when no Vulkan block declares one, which is a shader that would
// not have baked either.
static bool gvkExtractShaderVersion(const std::string& source, int& outVersion,
		shaderc_profile& outProfile, std::string& outStripped) {
	outVersion = 0;
	outProfile = shaderc_profile_core;
	outStripped.clear();
	outStripped.reserve(source.size());

	bool invulkanblock = false;
	std::stringstream reader(source);
	std::string line;
	while(std::getline(reader, line)) {
		if(line.rfind("#if", 0) == 0) {
			// Any marker beginning with VULKAN: the coloured-geometry shader splits
			// its Vulkan half into VULKAN2D and VULKAN3D, and both name the same
			// version.
			invulkanblock = line.find("VULKAN") != std::string::npos;
		} else if(line.rfind("#else", 0) == 0 || line.rfind("#endif", 0) == 0) {
			invulkanblock = false;
		}
		if(line.rfind("#version", 0) == 0) {
			if(invulkanblock && outVersion == 0) {
				std::stringstream parts(line.substr(8));
				std::string profile;
				parts >> outVersion >> profile;
				if(profile == "es") outProfile = shaderc_profile_es;
				else if(profile == "compatibility") outProfile = shaderc_profile_compatibility;
				else outProfile = shaderc_profile_core;
			}
			// Blank rather than dropped, so a compiler error still names the line the
			// shader author is looking at.
			outStripped += '\n';
			continue;
		}
		outStripped += line;
		outStripped += '\n';
	}
	return outVersion != 0;
}

bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		const std::string& define, std::vector<uint32_t>& spirv) {
	const std::string path = gvkShaderSourceDir() + fileName;
	std::ifstream file(path);
	if(!file.is_open()) {
		gLoge("gVKShaderCompiler") << "Could not open the shader source " << path;
		return false;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	const std::string rawsource = buffer.str();

	int version = 0;
	shaderc_profile profile = shaderc_profile_core;
	std::string source;
	if(!gvkExtractShaderVersion(rawsource, version, profile, source)) {
		gLoge("gVKShaderCompiler") << path << " declares no #version inside an #if VULKAN block.";
		return false;
	}

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
	options.SetForcedVersionProfile(version, profile);
	if(!define.empty()) options.AddMacroDefinition(define, "1");

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, path.c_str(), options);
	if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
		gLoge("gVKShaderCompiler") << fileName << ": " << result.GetErrorMessage();
		return false;
	}
	spirv.assign(result.cbegin(), result.cend());
	return !spirv.empty();
}

bool gvkCompileShaderSource(const std::string& source, VkShaderStageFlagBits stage,
		const std::string& debugName, std::vector<uint32_t>& spirv) {
	if(source.empty()) return false;

	shaderc_shader_kind kind;
	switch(stage) {
	case VK_SHADER_STAGE_VERTEX_BIT: kind = shaderc_vertex_shader; break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: kind = shaderc_fragment_shader; break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: kind = shaderc_geometry_shader; break;
	case VK_SHADER_STAGE_COMPUTE_BIT: kind = shaderc_compute_shader; break;
	default:
		gLoge("gVKShaderCompiler") << "Unsupported shader stage for " << debugName;
		return false;
	}

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	// No forced version here, unlike gvkCompileShaderFile above: this compiles an
	// application's shader, which gShader has already run its preprocessor over, so
	// the #version its dialect block declared is the first line of what arrives.
	// Keeps OpName and OpMemberName, which is what makes setFloat("name", v) able
	// to find anything. Optimisation still runs.
	options.SetGenerateDebugInfo();

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, debugName.c_str(), options);
	if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
		gLoge("gVKShaderCompiler") << debugName << ": " << result.GetErrorMessage();
		return false;
	}
	spirv.assign(result.cbegin(), result.cend());
	return !spirv.empty();
}

#else

bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		const std::string& define, std::vector<uint32_t>& spirv) {
	(void)fileName;
	(void)stage;
	(void)define;
	(void)spirv;
	return false;
}

bool gvkCompileShaderSource(const std::string& source, VkShaderStageFlagBits stage,
		const std::string& debugName, std::vector<uint32_t>& spirv) {
	(void)source;
	(void)stage;
	(void)debugName;
	(void)spirv;
	return false;
}

#endif /* GVK_RUNTIME_SHADERS */

#endif /* GVK_VULKAN */
