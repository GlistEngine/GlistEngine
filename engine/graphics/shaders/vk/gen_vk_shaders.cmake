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
	gvkspv_mesh3d_vert mesh3d.vert
	gvkspv_mesh3d_frag mesh3d.frag
	gvkspv_mesh3dpbr_vert mesh3dpbr.vert
	gvkspv_mesh3dpbr_frag mesh3dpbr.frag
	gvkspv_shadow3d_vert shadow3d.vert
	gvkspv_shadow3d_frag shadow3d.frag
	gvkspv_skybox3d_vert skybox3d.vert
	gvkspv_skybox3d_frag skybox3d.frag
	gvkspv_fbo_vert fbo.vert
	gvkspv_fbo_frag fbo.frag
	gvkspv_magnifier_frag magnifier.frag)

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

# Whether the committed SPIR-V still corresponds to these sources is a question
# about the sources, not about the clock and not about the compiler. Timestamps
# answer it wrongly in both directions: a fresh clone or a branch switch sets
# every file's modification time at once and in no useful order, so the rule
# fires with nothing changed - and then whichever glslc the developer happens to
# have rewrites the header, because two versions of the optimiser do not emit the
# same words for the same source. The result is a committed binary that changes
# from machine to machine and conflicts on every merge.
#
# So the header records a hash of the sources it was generated from, and the
# generator stops here when that hash still matches. Editing a shader changes it
# and the SPIR-V is regenerated; upgrading a compiler does not, and the committed
# bytes stay put. Pass -DFORCE=ON to regenerate anyway, which is what a
# deliberate toolchain upgrade wants.
set(GVK_HASH_INPUT "")
list(LENGTH GVK_SHADERS GVK_HASH_COUNT)
math(EXPR GVK_HASH_LAST "${GVK_HASH_COUNT} - 1")
foreach(i RANGE 0 ${GVK_HASH_LAST} 2)
	math(EXPR j "${i} + 1")
	list(GET GVK_SHADERS ${j} file)
	file(READ "${SHADER_DIR}/${file}" GVK_ONE_SOURCE)
	string(APPEND GVK_HASH_INPUT "${file}\n${GVK_ONE_SOURCE}")
endforeach()
# The generator itself is part of the input: change how the header is written and
# the header has to be rewritten.
file(READ "${CMAKE_CURRENT_LIST_FILE}" GVK_GENERATOR_SOURCE)
string(APPEND GVK_HASH_INPUT "${GVK_GENERATOR_SOURCE}")
string(SHA256 GVK_HASH "${GVK_HASH_INPUT}")
set(GVK_HASH_LINE "// source-sha256: ${GVK_HASH}")

if(NOT FORCE AND EXISTS "${OUT}")
	file(STRINGS "${OUT}" GVK_RECORDED_HASH REGEX "^// source-sha256: ")
	if(GVK_RECORDED_HASH STREQUAL GVK_HASH_LINE)
		message(STATUS "Vulkan shader sources unchanged; keeping the committed SPIR-V in ${OUT}")
		return()
	endif()
endif()

list(LENGTH GVK_SHADERS GVK_SHADER_COUNT)
math(EXPR GVK_LAST "${GVK_SHADER_COUNT} - 1")
foreach(i RANGE 0 ${GVK_LAST} 2)
	list(GET GVK_SHADERS ${i} name)
	math(EXPR j "${i} + 1")
	list(GET GVK_SHADERS ${j} file)

	# -mfmt=c emits the SPIR-V as a braced C initialiser; -o - sends it to stdout,
	# so no temporary file is needed. The flags match what the engine's runtime
	# compiler uses, so both paths produce the same code.
set(GLSLC_DEBUG_FLAGS)
if(file STREQUAL "magnifier.frag")
	list(APPEND GLSLC_DEBUG_FLAGS -g)
endif()

execute_process(
	COMMAND "${GLSLC}" --target-env=vulkan1.2 -O ${GLSLC_DEBUG_FLAGS}
		"${SHADER_DIR}/${file}" -mfmt=c -o -
	OUTPUT_VARIABLE spirv
	ERROR_VARIABLE glslcerror
	RESULT_VARIABLE glslcresult)
	if(NOT glslcresult EQUAL 0)
		message(FATAL_ERROR "glslc failed on ${file}:\n${glslcerror}")
	endif()

	string(APPEND GVK_HEADER "static const uint32_t ${name}[] =\n${spirv};\n\n")
endforeach()

string(APPEND GVK_HEADER "${GVK_HASH_LINE}\n")

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
