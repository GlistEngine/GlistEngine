# gen_vk_shaders.cmake
#
# Compiles the Vulkan 2D shaders in this directory to SPIR-V and writes them as
# uint32 arrays into engine/core/gVKShaders.h. That header is committed so the
# engine builds without a shader compiler present; this only has to run when a
# shader changes.
#
# engine/CMakeLists.txt runs it automatically whenever one of the sources is
# newer than the header. To run it by hand:
#
#   cmake -DGLSLC=<path to glslc> -DSHADER_DIR=<this directory> \
#         -DOUT=<path to engine/core/gVKShaders.h> -P gen_vk_shaders.cmake
#
# It is written in CMake rather than a shell script so that Windows regenerates
# the header the same way macOS and Linux do - a shell script left Windows
# silently shipping a stale gVKShaders.h.

if(NOT DEFINED GLSLC OR NOT DEFINED SHADER_DIR OR NOT DEFINED OUT)
	message(FATAL_ERROR "gen_vk_shaders.cmake needs -DGLSLC, -DSHADER_DIR and -DOUT")
endif()

# Array name followed by its source file. Add a pair here when a shader joins the
# 2D path; gVKPipeline.cpp reads the arrays by these names.
set(GVK_SHADERS
	gvkspv_color2d_vert color2d.vert
	gvkspv_color2d_frag color2d.frag
	gvkspv_image2d_vert image2d.vert
	gvkspv_image2d_frag image2d.frag
	gvkspv_color3d_vert color3d.vert
	gvkspv_color3d_frag color3d.frag
	gvkspv_shadow3d_vert shadow3d.vert
	gvkspv_shadow3d_frag shadow3d.frag)

set(GVK_HEADER "/*
 * gVKShaders.h
 *
 * SPIR-V for the Vulkan 2D shaders, generated from the .vert and .frag
 * sources in engine/graphics/shaders/vk by gen_vk_shaders.cmake.
 * Do not edit by hand: change the .vert/.frag source and rebuild.
 */

#pragma once

#include <cstdint>

")

list(LENGTH GVK_SHADERS GVK_SHADER_COUNT)
math(EXPR GVK_LAST "${GVK_SHADER_COUNT} - 1")
foreach(i RANGE 0 ${GVK_LAST} 2)
	list(GET GVK_SHADERS ${i} name)
	math(EXPR j "${i} + 1")
	list(GET GVK_SHADERS ${j} file)

	# -mfmt=c emits the SPIR-V as a braced C initialiser; -o - sends it to stdout,
	# so no temporary file is needed. The flags match what the engine's runtime
	# compiler uses, so both paths produce the same code.
	execute_process(
		COMMAND "${GLSLC}" --target-env=vulkan1.2 -O "${SHADER_DIR}/${file}" -mfmt=c -o -
		OUTPUT_VARIABLE spirv
		ERROR_VARIABLE glslcerror
		RESULT_VARIABLE glslcresult)
	if(NOT glslcresult EQUAL 0)
		message(FATAL_ERROR "glslc failed on ${file}:\n${glslcerror}")
	endif()

	string(APPEND GVK_HEADER "static const uint32_t ${name}[] =\n${spirv};\n\n")
endforeach()

# Only touch the header when the bytes actually change, so an unrelated rebuild
# does not keep restamping a committed file.
set(GVK_EXISTING "")
if(EXISTS "${OUT}")
	file(READ "${OUT}" GVK_EXISTING)
endif()
if(NOT GVK_EXISTING STREQUAL GVK_HEADER)
	file(WRITE "${OUT}" "${GVK_HEADER}")
	message(STATUS "Wrote ${OUT}")
else()
	message(STATUS "${OUT} is already up to date")
endif()
