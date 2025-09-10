cmake_minimum_required (VERSION 3.10.2)

if(NOT DEFINED GLIST_CXX_STANDARD)
	set(GLIST_CXX_STANDARD 14)
endif()
if(NOT DEFINED GLIST_CXX_STANDARD_REQUIRED)
	set(GLIST_CXX_STANDARD_REQUIRED True)
endif()

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
if(NOT APPLE)
	set(GLIST_SHARED_LINKER_FLAGS "-v -pthread -Wl,--gc-sections")
endif()
if(WIN32)
	set(GLIST_SHARED_LINKER_FLAGS "${GLIST_SHARED_LINKER_FLAGS} -mwindows")
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
#if(CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
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

foreach(pluginname ${PLUGINS})
	if(IOS)
        if(${pluginname} STREQUAL "gipIOS")
            set(IOS_PLUGIN_PRESENT 1)
			# We do not compile the gipIOS inside glistengine.
			continue()
        endif()
    endif()

    if(ANDROID)
        if(${pluginname} STREQUAL "gipAndroid")
            set(ANDROID_PLUGIN_PRESENT 1)
        endif()
    endif()
    include(${PLUGINS_DIR}/${pluginname}/CMakeLists.txt)
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
	${ENGINE_DIR}/graphics/gFog.cpp
	${ENGINE_DIR}/core/gObject.cpp
	${ENGINE_DIR}/core/gRenderer.cpp
	${ENGINE_DIR}/core/gGLRenderEngine.cpp
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
	${ENGINE_DIR}/graphics/gFont.cpp
	${ENGINE_DIR}/graphics/gMaterial.cpp
	${ENGINE_DIR}/graphics/gMesh.cpp
	${ENGINE_DIR}/graphics/gSkinnedMesh.cpp
	${ENGINE_DIR}/graphics/gModel.cpp
	${ENGINE_DIR}/graphics/gModelAnimator.cpp
	${ENGINE_DIR}/graphics/gMorphingMesh.cpp
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
	${ENGINE_DIR}/animation/gSpriteAnimation.cpp
	${ENGINE_DIR}/animation/gSpriteAnimator.cpp
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
	${ENGINE_DIR}/ui/gGUIToolbarButton.cpp
	${ENGINE_DIR}/ui/gGUIProgressBar.cpp
	${ENGINE_DIR}/ui/gGUIRadioButton.cpp
	${ENGINE_DIR}/ui/gGUIBitmap.cpp
	${ENGINE_DIR}/ui/gGUISlider.cpp
	${ENGINE_DIR}/ui/gGUIImageButton.cpp
	${ENGINE_DIR}/ui/gGUIColorbox.cpp
	${ENGINE_DIR}/ui/gGUIButton.cpp
	${ENGINE_DIR}/ui/gGUISizer.cpp
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
	${ENGINE_DIR}/ui/base/tinyfiledialogs.c
	${ENGINE_DIR}/base/gBaseGUIObject.cpp
	${ENGINE_DIR}/core/gGUIManager.cpp
	${ENGINE_DIR}/core/gAppObject.cpp
	${ENGINE_DIR}/base/gBaseComponent.cpp
	${ENGINE_DIR}/base/gBasePlugin.cpp
	${ENGINE_DIR}/media/gFmodSound.cpp
	${ENGINE_DIR}/media/gSound.cpp
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
	${ENGINE_DIR}/graphics/posteffects/gKernelEffect.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gBlur.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gSharpen.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gEdgeDetection.cpp
)

if(DEFINED TRACY_ENABLE)
	list(PREPEND SRC_FILES
			${ENGINE_DIR}/utils/gTracy.cpp
	)
endif()

if(WIN32)
	list(APPEND SRC_FILES
		${ENGINE_DIR}/utils/gWindows.cpp
	)
endif()

if(NOT (ANDROID OR IOS))
	list(APPEND SRC_FILES
		${MINGW64_DIR}/lib/sqlite3.c
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

if(APPLE)
    target_link_options(GlistEngine PRIVATE ${GLIST_SHARED_LINKER_FLAGS} -pthread)
else()
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GLIST_SHARED_LINKER_FLAGS}")
endif()

list(APPEND GLIST_CXX_FLAGS_DEBUG ${GLIST_CXX_FLAGS_COMMON} ${GLIST_CXX_FLAGS_DEBUG})
list(APPEND GLIST_CXX_FLAGS_RELEASE ${GLIST_CXX_FLAGS_COMMON} ${GLIST_CXX_FLAGS_RELEASE})
target_compile_options(
    GlistEngine
        PRIVATE
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:${GLIST_CXX_FLAGS_DEBUG}>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:${GLIST_CXX_FLAGS_RELEASE}>
)

########## INCLUDE DIRECTORIES ##########
set(INC_DIRS
	${LIBS_DIR}/stb/include
	${LIBS_DIR}/utf8/include
	${LIBS_DIR}/ghc/include
	${LIBS_DIR}/miniaudio/include
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

if(DEFINED TRACY_ENABLE)
	target_compile_definitions(GlistEngine PUBLIC TRACY_ENABLE)
	list(APPEND INC_DIRS
			${LIBS_DIR}/tracy/public
	)
endif()

if(WIN32)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include/c++/v1
		${MINGW64_DIR}/include
		${MINGW64_DIR}/include/freetype2
		${MINGW64_DIR}/include/sqlite
		${MINGW64_DIR}/include/curl
		${MINGW64_DIR}/include/utf8
		${MINGW64_DIR}/include/harfbuzz
	)
elseif(ANDROID)
    # These are added by gipAndroid plugin
    # Do not remove the if, otherwise linux libraries will be linked/included.
elseif(IOS)
    # These are added by gipIOS plugin
    # Do not remove the if, otherwise linux libraries will be linked/included.
elseif(LINUX)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include
		${MINGW64_DIR}/include/freetype2
		${MINGW64_DIR}/include/sqlite
		# TODO: FFMPEG
		
		/usr/include/freetype2
		/usr/include
		/usr/local/lib
	)
elseif(APPLE)
  	list(APPEND INC_DIRS
  		/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
  		${SYS_INC_PATH}
  		${SYS_INC_PATH}/freetype2
		${MINGW64_DIR}/include/sqlite
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

  	if(${DEVICEARCHITECTURE} STREQUAL "arm64")
		if(${ENGINE_USE_SYMLINKS})
			list(APPEND INC_DIRS
					${BREW_PATH}/llvm/include/c++/v1
			)
		else()
			# For backwards compatibility
			list(APPEND INC_DIRS
					${BREW_PATH}/llvm/${LLVM_VER}/include/c++/v1
			)
		endif()
  	endif()
endif(WIN32)


target_include_directories(GlistEngine PUBLIC ${INC_DIRS})
target_sources(GlistEngine PRIVATE ${ENGINE_INCLUDES})

#message(WARNING "PLUGIN_LIBDIRS: ${PLUGIN_LIBDIRS}")
FOREACH(PLIBDIR ${PLUGIN_LIBDIRS})
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${PLIBDIR}")
ENDFOREACH()

########## FIND LIBRARIES ##########
if(WIN32)
	link_directories(
			${MINGW64_DIR}/bin
			${MINGW64_DIR}/lib
			${PLUGIN_LINKDIRS}
	)
elseif(ANDROID)
    # These are added by gipAndroid plugin
    # Do not remove the if, otherwise linux libraries will be linked/included.
	if(NOT DEFINED ANDROID_PLUGIN_PRESENT)
        message(FATAL_ERROR
        " gipAndroid was not found in the active plugins list while trying to create an Android app. \n\n"
        "This might be caused by not passing gipAndroid to the GlistApp's used plugins\n"
        "list. You may also not have the gipAndroid plugin installed in the correct\n"
        "folder and not with the correct name. Check if you have installed the plugin\n"
        "under dev/glist/glistplugins."
        )
	endif()
elseif(IOS)
    if(NOT DEFINED IOS_PLUGIN_PRESENT)
		message(FATAL_ERROR
			" gipIOS was not found in the active plugins list while trying to create an IOS app. \n\n"
			"This might be caused by not passing gipIOS to the GlistApp's used plugins\n"
			"list. You may also not have the gipIOS plugin installed in the correct\n"
			"folder and not with the correct name. Check if you have installed the plugin\n"
			"under dev/glist/glistplugins."
		)
    endif()
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
    set(APPLE_LIBDIR "/opt/homebrew/lib")
    find_package(PkgConfig REQUIRED)
    set(ENV{PKG_CONFIG_PATH} "${APPLE_LIBDIR}/pkgconfig")
    
    pkg_check_modules(curl     REQUIRED IMPORTED_TARGET libcurl)
    pkg_check_modules(freetype REQUIRED IMPORTED_TARGET freetype2)
    pkg_check_modules(glfw     REQUIRED IMPORTED_TARGET glfw3)
    pkg_check_modules(assimp   REQUIRED IMPORTED_TARGET assimp)
    pkg_check_modules(glew     REQUIRED IMPORTED_TARGET glew)

    find_library(Lib_GL GL HINTS /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries)

    if(${ENGINE_USE_SYMLINKS})
        pkg_check_modules(crypto REQUIRED IMPORTED_TARGET libcrypto)
        set(Lib_crypto PkgConfig::crypto)
    else()
        # For backwards compatibility
        find_library(Lib_crypto crypto HINTS ${BREW_PATH}/openssl@${OPENSSL_VER_SHORT}/${OPENSSL_VER_FULL}/lib)
    endif()
endif (WIN32)


########## LINK LIBRARIES ##########
set(LINK_LIBS
	${PLUGIN_LINKLIBS}
)

if(WIN32)
	list(APPEND LINK_LIBS
		assimp
		freetype
		curl
		libcrypto
		glfw3
		opengl32
		glew32
		dbghelp
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
	if(DEFINED TRACY_ENABLE)
		list(APPEND LINK_LIBS
				ws2_32      # Windows Sockets API
				wsock32     # Additional socket functions
				${LIB_TRACY}
		)
	endif()
elseif(ANDROID)
    # These are added by gipAndroid plugin
    # Do not remove the if, otherwise linux libraries will be linked/included.
elseif(IOS)
    # These are added by gipIOS plugin
    # Do not remove the if, otherwise MacOS libraries will be linked/included.
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
		${Lib_crypto}
#		${Lib_cpp}
	# TODO: FFMPEG
	)
endif(WIN32)

# add lib dependencies
target_link_libraries(GlistEngine
    PUBLIC
        ${LINK_LIBS}
)
