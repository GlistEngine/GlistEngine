cmake_minimum_required (VERSION 3.13)

if(NOT DEFINED GLIST_CXX_STANDARD)
	set(GLIST_CXX_STANDARD 17)
endif()
if(NOT DEFINED GLIST_CXX_STANDARD_REQUIRED)
	set(GLIST_CXX_STANDARD_REQUIRED True)
endif()

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
	cmake_policy(SET CMP0025 NEW)
endif()

set(CMAKE_CXX_STANDARD ${GLIST_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ${GLIST_CXX_STANDARD_REQUIRED})

# Configure compiler and linker as you need
set(GLIST_CXX_FLAGS_COMMON
    -Wuninitialized
    -Wmultichar
    -ffunction-sections
    -fdata-sections
    -fexceptions
    -frtti
    -fpermissive
    -fPIC
)

# Only add libc++ if using Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	list(INSERT GLIST_CXX_FLAGS_COMMON 0 -stdlib=libc++)
endif()

list(APPEND GLIST_CXX_FLAGS_DEBUG -DDEBUG -O0 -g)
list(APPEND GLIST_CXX_FLAGS_RELEASE -DEIGEN_NO_CUDA -DNDEBUG -O2)
if(NOT DEFINED GLIST_SHARED_LINKER_FLAGS)
	set(GLIST_SHARED_LINKER_FLAGS )
endif()

if(APPLE)
	list(APPEND GLIST_SHARED_LINKER_FLAGS -pthread)
else()
	list(APPEND GLIST_SHARED_LINKER_FLAGS -v -pthread -Wl,--gc-sections)
endif()

if(WIN32)
	list(APPEND GLIST_SHARED_LINKER_FLAGS -mwindows)
endif()

##################################################
# DO NOT CHANGE BELOW
##################################################

########## BUILD VARIABLES ##########
if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
	set(SYS_NAME "linux")
endif()
    
if(WIN32)
    list(APPEND GLIST_CXX_FLAGS_RELEASE "-march=x86-64")
	list(APPEND GLIST_CXX_FLAGS_DEBUG "-march=x86-64")
	set(SYS_NAME "win64")
endif(WIN32)

if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE SHARED)
endif()
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

option(ENABLE_TRACY "Enables the tracy profiler." OFF)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(ENABLE_BOOST_WORKAROUND ON)

if(NOT DEFINED ENGINE_DIR)
    set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()
if(NOT DEFINED TOP_DIR)
    set(TOP_DIR ${ENGINE_DIR}/../..)
endif()
if(NOT DEFINED MAIN_DIR)
    set(MAIN_DIR ${ENGINE_DIR}/..)
endif()
if(NOT DEFINED LIBS_DIR)
    set(LIBS_DIR ${MAIN_DIR}/libs)
endif()
if(NOT DEFINED PLUGINS_DIR)
    set(PLUGINS_DIR ${TOP_DIR}/glistplugins)
endif()
if(NOT DEFINED ZBIN_DIR)
	set(ZBIN_DIR ${TOP_DIR}/zbin)
endif()
if(NOT DEFINED SYSZBIN_DIR)
	set(SYSZBIN_DIR ${ZBIN_DIR}/glistzbin-${SYS_NAME})
endif()
if(NOT DEFINED MINGW64_DIR)
if(WIN32)
	set(MINGW64_DIR ${SYSZBIN_DIR}/clang64)
endif(WIN32)
if(NOT WIN32)
	set(MINGW64_DIR ${SYSZBIN_DIR}/mingw64)
endif(NOT WIN32)
endif()

if(ENABLE_TRACY)
	message(STATUS "Tracy profiling enabled for ${CMAKE_BUILD_TYPE} build")
	include(FetchContent)
	FetchContent_Declare(
	    tracy
	    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
	    GIT_TAG v0.12.2  # or whatever version you want
	    GIT_SHALLOW TRUE
	    GIT_PROGRESS TRUE
	)
	FetchContent_MakeAvailable(tracy)
	FetchContent_GetProperties(tracy)
	if(tracy_POPULATED)
		message(STATUS "Tracy successfully downloaded and configured")
		set(TRACY_ENABLE ON CACHE BOOL "Tracy profiling enabled" FORCE)
		# Optional: Verify Tracy target exists
		if(TARGET Tracy::TracyClient)
			message(STATUS "Tracy::TracyClient target available")
			set(LIB_TRACY Tracy::TracyClient)
		elseif(TARGET tracy)
			message(STATUS "tracy target available")
			set(LIB_TRACY tracy)
		else()
			message(WARNING "Tracy downloaded but no target found")
			set(TRACY_ENABLE OFF CACHE BOOL "Tracy target not found" FORCE)
		endif()
	else()
		message(WARNING "Failed to download/configure Tracy - profiling disabled")
		set(TRACY_ENABLE OFF CACHE BOOL "Tracy download failed" FORCE)
	endif()
else()
	set(TRACY_ENABLE OFF CACHE BOOL "Tracy was not enabled" FORCE)
endif()

set(GENERATED_HEADERS_DIR "${CMAKE_BINARY_DIR}/generated")

#configure_file( ${ENGINE_DIR}/gConfig.h.in ${CMAKE_BINARY_DIR}/generated/gConfig.h )

########## READ PLUGIN CMAKELISTS ##########
if(NOT DEFINED PLUGINS)
    set(PLUGINS)
endif()
set(PLUGIN_SRCS)
set(PLUGIN_INCLUDES)
set(PLUGIN_LINKDIRS)
set(PLUGIN_LINKLIBS)
set(PLUGIN_LIBDIRS)

# Enable these if you want its plugins to be included automatically on these platforms.
##if(IOS)
##	# If compiling for iOS, we auto-add gipIOS, you don't have to.
##	list(APPEND PLUGINS gipIOS)
##endif()
##if(ANDROID)
##	# If compiling for Android, we auto-add gipAndroid, you don't have to.
##	list(APPEND PLUGINS gipAndroid)
##endif()
##if(EMSCRIPTEN)
##	# If emscripten is added, we auto-add gipWebGL, you don't have to.
##	list(APPEND PLUGINS gipWebGL)
##endif()

list(REMOVE_DUPLICATES PLUGINS)

foreach(pluginname ${PLUGINS})
	set(PLUGIN_CMAKE_PATH "${PLUGINS_DIR}/${pluginname}/CMakeLists.txt")
	if(NOT EXISTS ${PLUGIN_CMAKE_PATH})
		message(FATAL_ERROR
			"Plugin '${pluginname}' was not found at:\n"
			"  ${PLUGINS_DIR}/${pluginname}\n\n"
			"Check that the plugin is installed under ${PLUGINS_DIR}\n"
			"with exactly that folder name, and that it contains a CMakeLists.txt.\n"
			"Plugins can be downloaded from https://github.com/GlistPlugins/"
		)
	endif()
	# Include the plugin cmake
	include(${PLUGIN_CMAKE_PATH})
endforeach()



########## CPP SOURCE FILES ##########
set(SRC_FILES
	${PLUGIN_SRCS}
	${ENGINE_DIR}/graphics/particles/gFireworksParticles.cpp
	${ENGINE_DIR}/graphics/particles/gFireParticles.cpp
	${ENGINE_DIR}/base/gBaseParticles.cpp
	${ENGINE_DIR}/types/gColor.cpp
	${ENGINE_DIR}/core/gGUIAppThread.cpp
	${ENGINE_DIR}/utils/gThread.cpp
	${ENGINE_DIR}/utils/gUtils.cpp
	${ENGINE_DIR}/utils/gUUID.cpp
	${ENGINE_DIR}/utils/gFile.cpp
	${ENGINE_DIR}/utils/gHttpFile.cpp
	${ENGINE_DIR}/graphics/gShader.cpp
	${ENGINE_DIR}/graphics/gBezier.cpp
	${ENGINE_DIR}/graphics/gPath.cpp
	${ENGINE_DIR}/graphics/gFog.cpp
	${ENGINE_DIR}/core/gObject.cpp
	${ENGINE_DIR}/core/gRenderer.cpp
	${ENGINE_DIR}/core/gGLRenderEngine.cpp
	${ENGINE_DIR}/core/gVKRenderEngine.cpp
	${ENGINE_DIR}/core/gVKSwapchain.cpp
	${ENGINE_DIR}/core/gVKRenderTarget.cpp
	${ENGINE_DIR}/core/gVKCommands.cpp
	${ENGINE_DIR}/core/gVKSync.cpp
	${ENGINE_DIR}/core/gVKFrame.cpp
	${ENGINE_DIR}/core/gVKBuffer.cpp
	${ENGINE_DIR}/core/gVKMeshBuffer.cpp
	${ENGINE_DIR}/core/gVKUniform.cpp
	${ENGINE_DIR}/core/gVKShadow.cpp
	${ENGINE_DIR}/core/gVKPipeline.cpp
	${ENGINE_DIR}/core/gVKReflect.cpp
	${ENGINE_DIR}/core/gVKShaderCompiler.cpp
	${ENGINE_DIR}/core/gVKDraw.cpp
	${ENGINE_DIR}/core/gVKTexture.cpp
	${ENGINE_DIR}/core/gRenderObject.cpp
	${ENGINE_DIR}/core/gGUIActionManager.cpp
	${ENGINE_DIR}/graphics/gTexture.cpp
	${ENGINE_DIR}/graphics/gGrid.cpp
	${ENGINE_DIR}/graphics/gImage.cpp
	${ENGINE_DIR}/graphics/gSkybox.cpp
	${ENGINE_DIR}/graphics/gNode.cpp
	${ENGINE_DIR}/graphics/gLight.cpp
	${ENGINE_DIR}/graphics/gShadowMap.cpp
	${ENGINE_DIR}/graphics/gCamera.cpp
	${ENGINE_DIR}/graphics/gCameraController.cpp
	${ENGINE_DIR}/graphics/gFont.cpp
	${ENGINE_DIR}/graphics/gBitmapFont.cpp
	${ENGINE_DIR}/graphics/gMaterial.cpp
	${ENGINE_DIR}/graphics/gMesh.cpp
	${ENGINE_DIR}/graphics/gSkinnedMesh.cpp
	${ENGINE_DIR}/graphics/gModel.cpp
	${ENGINE_DIR}/graphics/gModelAnimator.cpp
	${ENGINE_DIR}/graphics/gMorphingMesh.cpp
	${ENGINE_DIR}/graphics/gTerrain.cpp
	${ENGINE_DIR}/utils/gLocalization.cpp
	${ENGINE_DIR}/utils/gDatabase.cpp
	${ENGINE_DIR}/utils/gAssetLoader.cpp
	${ENGINE_DIR}/utils/gAES.cpp
	${ENGINE_DIR}/graphics/primitives/gCross.cpp
	${ENGINE_DIR}/graphics/primitives/gArc.cpp
	${ENGINE_DIR}/graphics/primitives/gLine.cpp
	${ENGINE_DIR}/graphics/primitives/gCircle.cpp
	${ENGINE_DIR}/graphics/primitives/gRectangle.cpp
	${ENGINE_DIR}/graphics/primitives/gRoundedRectangle.cpp
	${ENGINE_DIR}/graphics/primitives/gPlane.cpp
	${ENGINE_DIR}/graphics/primitives/gBox.cpp
	${ENGINE_DIR}/graphics/primitives/gSphere.cpp
	${ENGINE_DIR}/graphics/primitives/gTriangle.cpp
	${ENGINE_DIR}/graphics/primitives/gCylinder.cpp
	${ENGINE_DIR}/graphics/primitives/gCone.cpp
	${ENGINE_DIR}/graphics/primitives/gTube.cpp
	${ENGINE_DIR}/graphics/gVbo.cpp
	${ENGINE_DIR}/graphics/gFbo.cpp
	${ENGINE_DIR}/graphics/gUbo.cpp
	${ENGINE_DIR}/graphics/gRay.cpp
	${ENGINE_DIR}/graphics/gBoundingBox.cpp
	${ENGINE_DIR}/graphics/gRect.cpp
	${ENGINE_DIR}/animation/gSpriteAnimator.cpp
	${ENGINE_DIR}/animation/gSpriteAnimation.cpp 
	${ENGINE_DIR}/animation/gTweenAnimation.cpp 
	${ENGINE_DIR}/ui/gGUITable.cpp
	${ENGINE_DIR}/ui/gGUISurface.cpp
	${ENGINE_DIR}/ui/gGUIGrid.cpp
	${ENGINE_DIR}/ui/gGUIColorPicker.cpp
	${ENGINE_DIR}/ui/gGUINotebook.cpp
	${ENGINE_DIR}/ui/gGUICandleStickChart.cpp
	${ENGINE_DIR}/ui/gGUIStatusBar.cpp
	${ENGINE_DIR}/ui/gGUILineGraph.cpp
	${ENGINE_DIR}/ui/gGUIContextMenu.cpp
	${ENGINE_DIR}/ui/gGUIRadarChart.cpp
	${ENGINE_DIR}/ui/gGUIDialogue.cpp
	${ENGINE_DIR}/ui/gGUITimebox.cpp
	${ENGINE_DIR}/ui/gGUIPieGraph.cpp
	${ENGINE_DIR}/ui/gGUIGraph.cpp
	${ENGINE_DIR}/ui/gGUINumberBox.cpp
	${ENGINE_DIR}/ui/gGUISwitchButton.cpp
	${ENGINE_DIR}/ui/gGUIListbox.cpp
	${ENGINE_DIR}/ui/gGUITreelist.cpp
	${ENGINE_DIR}/ui/gGUIDropdownList.cpp
	${ENGINE_DIR}/ui/gGUICheckbox.cpp
	${ENGINE_DIR}/ui/gGUIColumnChart.cpp
	${ENGINE_DIR}/ui/gGUITextbox.cpp
	${ENGINE_DIR}/ui/gGUIText.cpp
	${ENGINE_DIR}/ui/gGUITooltipText.cpp
	${ENGINE_DIR}/ui/gGUIDivider.cpp
	${ENGINE_DIR}/ui/gGUILine.cpp
	${ENGINE_DIR}/ui/gGUIToolbarButton.cpp
	${ENGINE_DIR}/ui/gGUIProgressBar.cpp
	${ENGINE_DIR}/ui/gGUIRadioButton.cpp
	${ENGINE_DIR}/ui/gGUIBitmap.cpp
	${ENGINE_DIR}/ui/gGUISlider.cpp
	${ENGINE_DIR}/ui/gGUIImageButton.cpp
	${ENGINE_DIR}/ui/gGUIColorbox.cpp
	${ENGINE_DIR}/ui/gGUIButton.cpp
	${ENGINE_DIR}/ui/gGUISizer.cpp
	${ENGINE_DIR}/ui/gGUISizerSpace.cpp
	${ENGINE_DIR}/ui/gGUINavigation.cpp
	${ENGINE_DIR}/ui/gGUIToolbar.cpp
	${ENGINE_DIR}/ui/gGUIMenubar.cpp
	${ENGINE_DIR}/ui/gGUIPane.cpp
	${ENGINE_DIR}/ui/gGUIPanel.cpp
	${ENGINE_DIR}/ui/gGUIScrollable.cpp
	${ENGINE_DIR}/ui/gGUIFrame.cpp
	${ENGINE_DIR}/ui/gGUIControl.cpp
	${ENGINE_DIR}/ui/gGUIContainer.cpp
	${ENGINE_DIR}/ui/gGUIForm.cpp
	${ENGINE_DIR}/ui/gGUIResources.cpp
	${ENGINE_DIR}/ui/gGUIDate.cpp
	${ENGINE_DIR}/ui/gGUIBoxPlot.cpp
	${ENGINE_DIR}/ui/base/tinyfiledialogs.c
	${ENGINE_DIR}/base/gBaseGUIObject.cpp
	${ENGINE_DIR}/core/gGUIManager.cpp
	${ENGINE_DIR}/core/gAppObject.cpp
	${ENGINE_DIR}/base/gBaseComponent.cpp
	${ENGINE_DIR}/base/gBasePlugin.cpp
	${ENGINE_DIR}/core/gInputManager.cpp
	${ENGINE_DIR}/core/gInputPlayer.cpp
	${ENGINE_DIR}/events/gEventHook.cpp
	${ENGINE_DIR}/media/gSound.cpp
	${ENGINE_DIR}/media/gSoundRecorder.cpp
	${ENGINE_DIR}/base/gBaseApp.cpp
	${ENGINE_DIR}/base/gBaseCanvas.cpp
	${ENGINE_DIR}/core/gBaseWindow.cpp
	${ENGINE_DIR}/core/gCanvasManager.cpp
	${ENGINE_DIR}/core/gAppManager.cpp
	${ENGINE_DIR}/core/gPostProcessManager.cpp
	${ENGINE_DIR}/base/gBasePostProcess.cpp
	${ENGINE_DIR}/graphics/posteffects/gInversion.cpp
	${ENGINE_DIR}/graphics/posteffects/gGrayscale.cpp
	${ENGINE_DIR}/graphics/posteffects/gSepiatone.cpp
	${ENGINE_DIR}/graphics/posteffects/gGrain.cpp
	${ENGINE_DIR}/graphics/posteffects/gDynamicGrain.cpp
	${ENGINE_DIR}/graphics/posteffects/gColorAdjustments.cpp
	${ENGINE_DIR}/graphics/posteffects/gColorChannelMixer.cpp
	${ENGINE_DIR}/graphics/posteffects/gSoftFocus.cpp
	${ENGINE_DIR}/graphics/posteffects/gVignette.cpp
	${ENGINE_DIR}/graphics/posteffects/gChromaticAberration.cpp
	${ENGINE_DIR}/graphics/posteffects/gHighContrast.cpp
	${ENGINE_DIR}/graphics/posteffects/gColorCurves.cpp
	${ENGINE_DIR}/graphics/posteffects/gShadowsMidtonesHighlights.cpp
	${ENGINE_DIR}/graphics/posteffects/gWhiteBalance.cpp
	${ENGINE_DIR}/graphics/posteffects/gTiltShift.cpp
	${ENGINE_DIR}/graphics/posteffects/gBloom.cpp
	${ENGINE_DIR}/graphics/posteffects/gBloomPyramid.cpp
	${ENGINE_DIR}/graphics/posteffects/gFXAA.cpp
	${ENGINE_DIR}/graphics/posteffects/gKernelEffect.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gBlur.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gSharpen.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gEdgeDetection.cpp
	${LIBS_DIR}/sqlite/lib/sqlite3.c
)

if(TRACY_ENABLE)
	list(PREPEND SRC_FILES
			${ENGINE_DIR}/utils/gTracy.cpp
	)
endif()

if(WIN32)
	list(APPEND SRC_FILES
		${ENGINE_DIR}/utils/gWindows.cpp
	)
endif()

# miniaudio includes AVFoundation on iOS, so this translation unit must accept
# Objective-C declarations. Keep the override platform-local; other targets
# continue compiling gSound.cpp as ordinary C++.
if(IOS)
	set_source_files_properties(${ENGINE_DIR}/media/gSound.cpp PROPERTIES
		LANGUAGE OBJCXX
		XCODE_EXPLICIT_FILE_TYPE "sourcecode.cpp.objcpp"
	)
endif()

# Window is defined by the plugin
if(NOT (ANDROID OR IOS OR EMSCRIPTEN))
	list(APPEND SRC_FILES
			${ENGINE_DIR}/core/backward.cpp
			${ENGINE_DIR}/core/gGLFWWindow.cpp
	)
endif()

set(SHADERS_DIR "${ENGINE_DIR}/graphics/shaders" CACHE STRING "Engine shaders directory")
set(GENERATED_SHADERS_DIR "${GENERATED_HEADERS_DIR}/graphics/shaders" CACHE STRING "Generated shaders directory")
file(MAKE_DIRECTORY ${GENERATED_SHADERS_DIR})
file(GLOB_RECURSE SHADER_FILES "${SHADERS_DIR}/*.glsl")

# If user or another plugin haven't defined a custom executable for the tool
if(NOT DEFINED SHADERTOHEADER_EXEC_PATH)
	add_executable(GlistShaderToHeader ${ENGINE_DIR}/tools/ShaderToHeader.cpp)
	set(SHADERTOHEADER_EXEC_PATH $<TARGET_FILE:GlistShaderToHeader>)
endif()

# Add a dependency to ensure the header file is generated before other targets
foreach(SHADER_FILE ${SHADER_FILES})
	get_filename_component(SHADER_FILE_NAME ${SHADER_FILE} NAME_WE)
	set(GENERATED_HEADER_FILE "${GENERATED_SHADERS_DIR}/${SHADER_FILE_NAME}.h")

	add_custom_command(PRE_BUILD
			OUTPUT ${GENERATED_HEADER_FILE}
			COMMAND ${SHADERTOHEADER_EXEC_PATH} ${SHADER_FILE} ${GENERATED_HEADER_FILE}
			DEPENDS ${SHADER_FILE}
			COMMENT "Generating header file for ${SHADER_FILE} at ${GENERATED_HEADER_FILE}"
	)
	list(PREPEND SRC_FILES ${GENERATED_HEADER_FILE})
endforeach()

add_custom_target(
		generate_shader_header
		DEPENDS ${SHADER_FILES} ${GENERATED_HEADERS_DIR}
)

if(ANDROID OR IOS)
	# We have androidMain function that is
	# defined inside the GlistEngine but implemented by the
	# GlistApp. To have this behavior, library should be static.
	add_library(GlistEngine STATIC
		${SRC_FILES}
	)
else()
	add_library(GlistEngine ${BUILD_TYPE}
		${SRC_FILES}
	)
endif()

if(IOS)
	# gSound.cpp defines MINIAUDIO_IMPLEMENTATION, which pulls in AVFoundation
	# (Objective-C) via miniaudio's Core Audio backend, so it must be compiled
	# as Objective-C++ on iOS.
	set_source_files_properties(${ENGINE_DIR}/media/gSound.cpp PROPERTIES LANGUAGE OBJCXX)
endif()

target_link_options(GlistEngine PRIVATE ${GLIST_SHARED_LINKER_FLAGS})

list(APPEND GLIST_CXX_FLAGS_DEBUG ${GLIST_CXX_FLAGS_COMMON} ${GLIST_CXX_FLAGS_DEBUG})
list(APPEND GLIST_CXX_FLAGS_RELEASE ${GLIST_CXX_FLAGS_COMMON} ${GLIST_CXX_FLAGS_RELEASE})
target_compile_options(
    GlistEngine
        PRIVATE
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:${GLIST_CXX_FLAGS_DEBUG}>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:${GLIST_CXX_FLAGS_RELEASE}>
)


if(EMSCRIPTEN)
	add_link_options(
			-sFULL_ES3
            -sUSE_WEBGL2=1
            -sUSE_GLFW=3
            -sUSE_FREETYPE=1
			-sUSE_LIBPNG=1
			-sUSE_ZLIB=1
			-sUSE_SQLITE3
			-sUSE_HARFBUZZ=1
            -lGL
            -lglfw
            -sFORCE_FILESYSTEM
            -sGL_PREINITIALIZED_CONTEXT
			-sALLOW_MEMORY_GROWTH=1
			-sSTACK_SIZE=1049088
			-sFORCE_FILESYSTEM
			-sINITIAL_MEMORY=256MB
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_link_options(
                ## debug stuff
                -sASSERTIONS
                -sSAFE_HEAP=1
                -sSTACK_OVERFLOW_CHECK=2
                -g3
        )
    endif()
endif()


########## INCLUDE DIRECTORIES ##########
set(INC_DIRS
	${LIBS_DIR}/stb/include
	${LIBS_DIR}/utf8/include
	${LIBS_DIR}/ghc/include
	${LIBS_DIR}/miniaudio/include
	${LIBS_DIR}/sqlite/include
)

set(ENGINE_INCLUDES
    ${ENGINE_DIR}/ui
    ${ENGINE_DIR}/utils
    ${ENGINE_DIR}/types
    ${ENGINE_DIR}/media
    ${ENGINE_DIR}/graphics
    ${ENGINE_DIR}/graphics/primitives
    ${ENGINE_DIR}/graphics/posteffects
    ${ENGINE_DIR}/graphics/posteffects/kerneleffects
    ${ENGINE_DIR}/animation
    ${ENGINE_DIR}/ui/base
    ${ENGINE_DIR}/base
    ${ENGINE_DIR}/core
    ${ENGINE_DIR}/events
    ${PLUGIN_INCLUDES}
	${GENERATED_HEADERS_DIR}
)

list(APPEND INC_DIRS ${ENGINE_INCLUDES})

if(TRACY_ENABLE)
	target_compile_definitions(GlistEngine PUBLIC TRACY_ENABLE)
	list(APPEND INC_DIRS
			${LIBS_DIR}/tracy/public
	)
endif()

if(EMSCRIPTEN)
	# These are added by gipWebGL plugin
	# Do not remove this scope
elseif(ANDROID)
    # These are added by gipAndroid plugin
	# Do not remove this scope
elseif(IOS)
    # These are added by gipIOS plugin
	# Do not remove this scope
elseif(WIN32)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include/c++/v1
		${MINGW64_DIR}/include
		${MINGW64_DIR}/include/freetype2
		${MINGW64_DIR}/include/curl
		${MINGW64_DIR}/include/utf8
		${MINGW64_DIR}/include/harfbuzz
	)
elseif(LINUX)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include
		${MINGW64_DIR}/include/freetype2
		# TODO: FFMPEG

		/usr/include/freetype2
		/usr/include
		/usr/local/lib
	)
elseif(APPLE)
	# Resolve the active SDK at configure time so we work with either full
	# Xcode or just the Command Line Tools (xcrun returns whichever is set
	# as the active developer dir).
	execute_process(
		COMMAND xcrun --show-sdk-path
		OUTPUT_VARIABLE OSX_SDK_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
  	list(APPEND INC_DIRS
  		${OSX_SDK_PATH}/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
  		${SYS_INC_PATH}
  		${SYS_INC_PATH}/freetype2
		# TODO: FFMPEG
  	)
	if(DEFINED ENGINE_USE_SYMLINKS)
		list(APPEND INC_DIRS
				${BREW_PATH}/openssl@3/include
		)
	else()
		# For backwards compatibility
		list(APPEND INC_DIRS
				${BREW_PATH}/openssl@${OPENSSL_VER_SHORT}/${OPENSSL_VER_FULL}/include
		)
	endif()
endif(WIN32)


target_include_directories(GlistEngine PUBLIC ${INC_DIRS})
target_sources(GlistEngine PRIVATE ${ENGINE_INCLUDES})

#message(WARNING "PLUGIN_LIBDIRS: ${PLUGIN_LIBDIRS}")
foreach(PLIBDIR ${PLUGIN_LIBDIRS})
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${PLIBDIR}")
endforeach()

########## FIND LIBRARIES ##########
if(EMSCRIPTEN)
	# These are added by gipWebGL plugin
	# Do not remove this block
elseif(ANDROID)
	# These are added by gipAndroid plugin
	# Do not remove this block
elseif(IOS)
	# These are added by gipIOS plugin
	# Do not remove this block
elseif(WIN32)
	find_library(GLIST_VULKAN_LIBRARY
		NAMES vulkan-1
		PATHS "${MINGW64_DIR}/lib"
		NO_DEFAULT_PATH
	)
	if(EXISTS "${MINGW64_DIR}/include/vulkan/vulkan.h" AND GLIST_VULKAN_LIBRARY)
		set(Lib_Vulkan "${GLIST_VULKAN_LIBRARY}")
		target_compile_definitions(GlistEngine PRIVATE GLIST_HAS_VULKAN)
		# The validation layer ships with zbin, but its manifest does not sit in a
		# directory the loader searches on its own. Passing the path to the engine
		# keeps the layer usable on machines that have no Vulkan SDK installed.
		if(EXISTS "${MINGW64_DIR}/bin/VkLayer_khronos_validation.json")
			target_compile_definitions(GlistEngine PRIVATE
				GLIST_VK_LAYER_PATH="${MINGW64_DIR}/bin")
		endif()
	else()
		message(STATUS "Vulkan development files not found in zbin; Vulkan backend disabled")
	endif()
    link_directories(
            ${MINGW64_DIR}/bin
            ${MINGW64_DIR}/lib
            ${PLUGIN_LINKDIRS}
    )
elseif(LINUX)
	# TODO: FFMPEG
#	find_library(Lib_Freetype2 freetype HINTS /usr/lib/x86_64-linux-gnu)
	
#	execute_process(COMMAND 
#	    pkg-config --cflags freetype2
#	    OUTPUT_VARIABLE
#	    CL_TMP_VAR
#	    OUTPUT_STRIP_TRAILING_WHITESPACE)
#	string(STRIP ${CL_TMP_VAR} CL_VAR_1)
#	add_definitions(${CL_VAR_1})

	find_package(Vulkan QUIET)
	if(Vulkan_FOUND)
		set(Lib_Vulkan Vulkan::Vulkan)
		target_compile_definitions(GlistEngine PRIVATE GLIST_HAS_VULKAN)
	else()
		message(STATUS "Vulkan development files not found; Vulkan backend disabled")
	endif()
	
	add_library(glfw SHARED IMPORTED)
	set_target_properties(glfw PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libglfw.so)

	add_library(assimp SHARED IMPORTED)
	set_target_properties(assimp PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libassimp.so)
	
	add_library(libpng SHARED IMPORTED)
	set_target_properties(libpng PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libpng.so)
	
	add_library(freetype SHARED IMPORTED)
	set_target_properties(freetype PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libfreetype.so)
elseif(APPLE)
    find_package(PkgConfig REQUIRED)
    
    pkg_check_modules(curl     REQUIRED IMPORTED_TARGET libcurl)
    pkg_check_modules(freetype REQUIRED IMPORTED_TARGET freetype2)
    pkg_check_modules(glfw     REQUIRED IMPORTED_TARGET glfw3)
    pkg_check_modules(assimp   REQUIRED IMPORTED_TARGET assimp)
    pkg_check_modules(glew     REQUIRED IMPORTED_TARGET glew)
    # Vulkan on macOS is provided by MoltenVK via the Homebrew vulkan-loader,
    # resolved through pkg-config like the rest of the stack. The imported target
    # carries the header include dirs and links the loader.
    pkg_check_modules(vulkan QUIET IMPORTED_TARGET vulkan)
    if(vulkan_FOUND)
        set(Lib_Vulkan PkgConfig::vulkan)
        target_compile_definitions(GlistEngine PRIVATE GLIST_HAS_VULKAN)
    else()
        message(STATUS "Vulkan development files not found; Vulkan backend disabled")
    endif()

    find_library(Lib_GL GL HINTS ${OSX_SDK_PATH}/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries)

    if(${ENGINE_USE_SYMLINKS})
        pkg_check_modules(crypto REQUIRED IMPORTED_TARGET libcrypto)
        set(Lib_crypto PkgConfig::crypto)
    else()
        # For backwards compatibility
        find_library(Lib_crypto crypto HINTS ${BREW_PATH}/openssl@${OPENSSL_VER_SHORT}/${OPENSSL_VER_FULL}/lib)
    endif()
endif()


########## VULKAN SHADER HOT RELOAD ##########
# A development build can compile engine/graphics/shaders/vk/*.vert|frag to
# SPIR-V while the app runs, so saving a shader shows up without rebuilding.
# It needs libshaderc, which is not linked into a release build: there the
# shaders come from the SPIR-V committed in core/gVKShaders.h. Both paths run
# the same sources through the same compiler settings, so this only changes how
# quickly an edit is visible, never what gets rendered.
#
# On by default in Debug builds where shaderc is present. Multi-config
# generators leave CMAKE_BUILD_TYPE empty, so those need -DGLIST_VK_SHADER_HOT_RELOAD=ON.
option(GLIST_VK_SHADER_HOT_RELOAD "Compile the Vulkan 2D shaders at run time (needs shaderc)" OFF)
if(Lib_Vulkan AND (GLIST_VK_SHADER_HOT_RELOAD OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
	set(GLIST_SHADERC_FOUND FALSE)
	if(WIN32)
		# Windows takes its libraries from zbin and nowhere else, exactly like the
		# Vulkan block above - NO_DEFAULT_PATH keeps a stray shaderc elsewhere on the
		# machine from being picked up instead of the one the toolchain ships.
		find_library(GLIST_SHADERC_LIBRARY
			NAMES shaderc_shared shaderc_combined shaderc
			PATHS "${MINGW64_DIR}/lib"
			NO_DEFAULT_PATH)
		find_path(GLIST_SHADERC_INCLUDE shaderc/shaderc.hpp
			PATHS "${MINGW64_DIR}/include"
			NO_DEFAULT_PATH)
		if(GLIST_SHADERC_LIBRARY AND GLIST_SHADERC_INCLUDE)
			target_include_directories(GlistEngine PRIVATE "${GLIST_SHADERC_INCLUDE}")
			target_link_libraries(GlistEngine PRIVATE "${GLIST_SHADERC_LIBRARY}")
			set(GLIST_SHADERC_FOUND TRUE)
			set(GLIST_SHADERC_ORIGIN "${GLIST_SHADERC_LIBRARY}")
		endif()
	else()
		find_package(PkgConfig QUIET)
		if(PKG_CONFIG_FOUND)
			pkg_check_modules(shaderc QUIET IMPORTED_TARGET shaderc)
		endif()
		if(shaderc_FOUND)
			target_link_libraries(GlistEngine PRIVATE PkgConfig::shaderc)
			set(GLIST_SHADERC_FOUND TRUE)
			set(GLIST_SHADERC_ORIGIN "shaderc ${shaderc_VERSION}")
		endif()
	endif()

	if(GLIST_SHADERC_FOUND)
		target_compile_definitions(GlistEngine PRIVATE
			GVK_RUNTIME_SHADERS
			GVK_VK_SHADER_SOURCE_DIR="${ENGINE_DIR}/graphics/shaders/vk")
		message(STATUS "Vulkan shader hot reload enabled (${GLIST_SHADERC_ORIGIN})")
	else()
		message(STATUS "shaderc not found; Vulkan shaders come from the committed SPIR-V")
	endif()
endif()


########## VULKAN SHADER SPIR-V ##########
# core/gVKShaders.h holds the SPIR-V the engine ships with, and is committed so
# the build needs no shader compiler. Keeping it in step by hand is a step that
# gets forgotten, so when glslc is available the header is regenerated whenever
# one of the shader sources is newer than it. Without glslc nothing runs and the
# committed header is used as-is, which is what keeps this off the critical path
# for anyone not editing shaders.
#
# This is separate from hot reload: reload shows an edit in the running app,
# this makes sure the edit reaches the committed SPIR-V before it is pushed.
if(Lib_Vulkan)
	if(WIN32)
		# Same rule as the libraries: glslc comes from zbin, not from wherever a
		# Vulkan SDK may have put one.
		find_program(GLIST_GLSLC glslc PATHS "${MINGW64_DIR}/bin" NO_DEFAULT_PATH)
	else()
		find_program(GLIST_GLSLC glslc HINTS /opt/homebrew/bin /usr/local/bin)
	endif()
	if(GLIST_GLSLC)
		set(GVK_SHADER_DIR ${ENGINE_DIR}/graphics/shaders/vk)
		# The generator is a CMake script rather than a shell script so this works
		# the same on Windows, macOS and Linux.
		add_custom_command(
			OUTPUT ${ENGINE_DIR}/core/gVKShaders.h
			COMMAND ${CMAKE_COMMAND}
				-DGLSLC=${GLIST_GLSLC}
				-DSHADER_DIR=${GVK_SHADER_DIR}
				-DOUT=${ENGINE_DIR}/core/gVKShaders.h
				-P ${GVK_SHADER_DIR}/gen_vk_shaders.cmake
			DEPENDS
				${GVK_SHADER_DIR}/color2d.vert
				${GVK_SHADER_DIR}/color2d.frag
				${GVK_SHADER_DIR}/image2d.vert
				${GVK_SHADER_DIR}/image2d.frag
				${GVK_SHADER_DIR}/mesh3d.vert
				${GVK_SHADER_DIR}/mesh3d.frag
				${GVK_SHADER_DIR}/mesh3dpbr.vert
				${GVK_SHADER_DIR}/mesh3dpbr.frag
				${GVK_SHADER_DIR}/shadow3d.vert
				${GVK_SHADER_DIR}/shadow3d.frag
				${GVK_SHADER_DIR}/skybox3d.vert
				${GVK_SHADER_DIR}/skybox3d.frag
				${GVK_SHADER_DIR}/gen_vk_shaders.cmake
			COMMENT "Regenerating gVKShaders.h from the Vulkan 2D shader sources"
			VERBATIM)
		add_custom_target(GlistVKShaders DEPENDS ${ENGINE_DIR}/core/gVKShaders.h)
		add_dependencies(GlistEngine GlistVKShaders)
	else()
		message(STATUS "glslc not found; core/gVKShaders.h will be used as committed")
	endif()
endif()


########## VULKAN VALIDATION LAYER MANIFEST ##########
# Some distributions - Homebrew's vulkan-validationlayers among them - ship a
# layer manifest whose "library_path" is a bare file name. The loader can then
# enumerate the layer (vulkaninfo lists it) but not load it, so vkCreateInstance
# fails with VK_ERROR_LAYER_NOT_PRESENT unless the library happens to sit on the
# default search path - which under Homebrew it does not.
#
# Rather than make every developer export DYLD_LIBRARY_PATH, a corrected copy of
# the manifest is written into the build tree with the library's absolute path,
# and gVKRenderEngine::initVulkan points VK_LAYER_PATH at it through the existing
# GLIST_VK_LAYER_PATH hook. Only Debug builds ask for the layer at all, and a
# manifest that already carries a usable path is left alone.
if(Lib_Vulkan AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	find_file(GLIST_VK_VALIDATION_MANIFEST VkLayer_khronos_validation.json
		PATHS
			/opt/homebrew/share/vulkan/explicit_layer.d
			/usr/local/share/vulkan/explicit_layer.d
			/usr/share/vulkan/explicit_layer.d
		NO_DEFAULT_PATH)
	find_library(GLIST_VK_VALIDATION_LIBRARY VkLayer_khronos_validation
		PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)
	if(GLIST_VK_VALIDATION_MANIFEST AND GLIST_VK_VALIDATION_LIBRARY)
		file(READ "${GLIST_VK_VALIDATION_MANIFEST}" GLIST_VK_MANIFEST_TEXT)
		# A value with no path separator is the broken case; anything else already
		# resolves, either absolutely or relative to the manifest's own directory.
		if(GLIST_VK_MANIFEST_TEXT MATCHES "\"library_path\"[ \t]*:[ \t]*\"[^/\"]+\"")
			string(REGEX REPLACE "(\"library_path\"[ \t]*:[ \t]*\")[^/\"]+(\")"
				"\\1${GLIST_VK_VALIDATION_LIBRARY}\\2"
				GLIST_VK_MANIFEST_TEXT "${GLIST_VK_MANIFEST_TEXT}")
			set(GLIST_VK_LAYER_DIR "${CMAKE_CURRENT_BINARY_DIR}/vklayers")
			file(WRITE "${GLIST_VK_LAYER_DIR}/VkLayer_khronos_validation.json" "${GLIST_VK_MANIFEST_TEXT}")
			target_compile_definitions(GlistEngine PRIVATE GLIST_VK_LAYER_PATH="${GLIST_VK_LAYER_DIR}")
			message(STATUS "Vulkan validation layer manifest rewritten with an absolute library path")
		endif()
	endif()
endif()


########## LINK LIBRARIES ##########
set(LINK_LIBS
	${PLUGIN_LINKLIBS}
)

if(EMSCRIPTEN)
	# These are added by gipWebGL plugin
	# Do not remove this block
elseif(ANDROID)
	# These are added by gipAndroid plugin
	# Do not remove this block
elseif(IOS)
	# These are added by gipIOS plugin
	# Do not remove this block
elseif(WIN32)
	list(APPEND LINK_LIBS
		assimp
		freetype
		curl
		libcrypto
		glfw3
		opengl32
		glew32
		${Lib_Vulkan}
		dbghelp
		Shcore
#		msvcr100
#		psapi
#		swscale-5
#		swresample-3
#		avutil-56
#		avformat-58
#		avfilter-7
#		avdevice-58
#		avcodec-58
	)
	if(TRACY_ENABLE)
		list(APPEND LINK_LIBS
				ws2_32      # Windows Sockets API
				wsock32     # Additional socket functions
				${LIB_TRACY}
		)
	endif()
elseif(LINUX)
	list(APPEND LINK_LIBS
		assimp
		freetype
		libpng
		curl
		crypto
		glfw
		GL
		GLEW
		${Lib_Vulkan}
		dl
	# TODO: FFMPEG
	)
elseif(APPLE)
	list(APPEND LINK_LIBS
		PkgConfig::assimp
		PkgConfig::freetype
		PkgConfig::curl
		PkgConfig::glfw
		${Lib_GL}
		PkgConfig::glew
		${Lib_Vulkan}
		${Lib_crypto}
#		${Lib_cpp}
	# TODO: FFMPEG
	)
endif()

# add lib dependencies
target_link_libraries(GlistEngine PUBLIC ${LINK_LIBS})
