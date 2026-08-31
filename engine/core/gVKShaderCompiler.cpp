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
#elif defined(GVK_RUNTIME_GLSLANG)
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>
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

#elif defined(GVK_RUNTIME_GLSLANG)

bool gvkCompileShaderFile(const std::string& fileName, VkShaderStageFlagBits stage,
		const std::string& define, std::vector<uint32_t>& spirv) {
	// Release builds use committed SPIR-V for the engine pipelines. glslang is
	// linked here for already-preprocessed gShader source, not for hot reload.
	(void)fileName;
	(void)stage;
	(void)define;
	(void)spirv;
	return false;
}

bool gvkCompileShaderSource(const std::string& source, VkShaderStageFlagBits stage,
		const std::string& debugName, std::vector<uint32_t>& spirv) {
	if(source.empty()) return false;

	glslang_stage_t language;
	switch(stage) {
	case VK_SHADER_STAGE_VERTEX_BIT: language = GLSLANG_STAGE_VERTEX; break;
	case VK_SHADER_STAGE_FRAGMENT_BIT: language = GLSLANG_STAGE_FRAGMENT; break;
	case VK_SHADER_STAGE_GEOMETRY_BIT: language = GLSLANG_STAGE_GEOMETRY; break;
	case VK_SHADER_STAGE_COMPUTE_BIT: language = GLSLANG_STAGE_COMPUTE; break;
	default:
		gLoge("gVKShaderCompiler") << "Unsupported shader stage for " << debugName;
		return false;
	}

	// Use glslang's C ABI here. Linux distributions commonly build glslang with
	// libstdc++ while GlistEngine uses libc++; passing C++ objects over that boundary
	// would make an otherwise valid Linux build fail to link. Initialisation is
	// process-wide and thread-safe after this static has run. Deliberately do not
	// finalise it: shaders may be constructed during static teardown, and the OS
	// releases the compiler library after the engine is gone.
	static const bool glslangready = glslang_initialize_process() != 0;
	if(!glslangready) {
		gLoge("gVKShaderCompiler") << "glslang initialisation failed for " << debugName;
		return false;
	}

	glslang_input_t input{};
	input.language = GLSLANG_SOURCE_GLSL;
	input.stage = language;
	input.client = GLSLANG_CLIENT_VULKAN;
	input.client_version = GLSLANG_TARGET_VULKAN_1_2;
	input.target_language = GLSLANG_TARGET_SPV;
	input.target_language_version = GLSLANG_TARGET_SPV_1_5;
	input.code = source.c_str();
	input.default_version = 450;
	input.default_profile = GLSLANG_CORE_PROFILE;
	input.force_default_version_and_profile = false;
	input.forward_compatible = false;
	input.messages = static_cast<glslang_messages_t>(
			GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT);
	input.resource = glslang_default_resource();

	glslang_shader_t* shader = glslang_shader_create(&input);
	if(shader == nullptr) {
		gLoge("gVKShaderCompiler") << "Could not create a glslang shader for " << debugName;
		return false;
	}
	if(!glslang_shader_preprocess(shader, &input) || !glslang_shader_parse(shader, &input)) {
		gLoge("gVKShaderCompiler") << debugName << ": " << glslang_shader_get_info_log(shader);
		glslang_shader_delete(shader);
		return false;
	}

	glslang_program_t* program = glslang_program_create();
	if(program == nullptr) {
		gLoge("gVKShaderCompiler") << "Could not create a glslang program for " << debugName;
		glslang_shader_delete(shader);
		return false;
	}
	glslang_program_add_shader(program, shader);
	if(!glslang_program_link(program, input.messages)) {
		gLoge("gVKShaderCompiler") << debugName << ": " << glslang_program_get_info_log(program);
		glslang_program_delete(program);
		glslang_shader_delete(shader);
		return false;
	}

	// The default generator retains OpName/OpMemberName, which reflection needs in
	// order to implement gShader's name-based uniform setters.
	glslang_program_SPIRV_generate(program, language);
	const size_t wordcount = glslang_program_SPIRV_get_size(program);
	if(wordcount > 0) {
		spirv.resize(wordcount);
		glslang_program_SPIRV_get(program, spirv.data());
	}
	const char* messages = glslang_program_SPIRV_get_messages(program);
	if(wordcount == 0 && messages != nullptr && messages[0] != '\0') {
		gLoge("gVKShaderCompiler") << debugName << ": " << messages;
	}

	glslang_program_delete(program);
	glslang_shader_delete(shader);
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
