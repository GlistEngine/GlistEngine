cmake_minimum_required (VERSION 3.10.2)

if(UNIX)
	EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE DEVICEARCHITECTURE )
	message( STATUS "Architecture: ${DEVICEARCHITECTURE}" )
endif()

# Configure compiler and linker as you need
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++14 -DNDEBUG -DENABLE_BOOST_WORKAROUND=ON -fuse-ld=gold -Wuninitialized -Wmultichar -ffunction-sections -fdata-sections -fexceptions -frtti -fpermissive -fPIC -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++14 -DDEBUG -DENABLE_BOOST_WORKAROUND=ON -fuse-ld=gold -Wuninitialized -Wmultichar -ffunction-sections -fdata-sections -fexceptions -frtti -fpermissive -fPIC -O0")
if(NOT APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -v -pthread -Wl,--no-as-needed -Wl,--gc-sections -Wl,--unresolved-symbols=report-all")
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
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(SYS_NAME "win64")
endif(WIN32)

if(APPLE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(SYS_NAME "macos")
endif(APPLE)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)


if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

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
	set(MINGW64_DIR ${SYSZBIN_DIR}/mingw64)
endif()
if(NOT DEFINED ANDROID_DIR)
	set(ANDROID_DIR ${ZBIN_DIR}/android)
endif()

if(APPLE)
    set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-11")
#    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
#    set(CMAKE_CXX_COMPILER "${MINGW64_DIR}/Cellar/gcc@7/7.5.0_3/bin/g++-7")
endif()


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
	include(${PLUGINS_DIR}/${pluginname}/CMakeLists.txt)
endforeach()


########## CPP SOURCE FILES ##########
set(SRC_FILES
	${PLUGIN_SRCS}
	${MINGW64_DIR}/lib/sqlite3.c
	${ENGINE_DIR}/types/gColor.cpp
	${ENGINE_DIR}/utils/gThread.cpp
	${ENGINE_DIR}/utils/gUtils.cpp
	${ENGINE_DIR}/utils/gFile.cpp
	${ENGINE_DIR}/utils/gHttpFile.cpp
	${ENGINE_DIR}/graphics/gShader.cpp
	${ENGINE_DIR}/graphics/gFog.cpp
	${ENGINE_DIR}/core/gObject.cpp
	${ENGINE_DIR}/core/gRenderer.cpp
	${ENGINE_DIR}/core/gRenderObject.cpp
	${ENGINE_DIR}/graphics/gTexture.cpp
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
	${ENGINE_DIR}/utils/gLocalization.cpp
	${ENGINE_DIR}/utils/gDatabase.cpp
	${ENGINE_DIR}/utils/gAssetLoader.cpp
	${ENGINE_DIR}/graphics/primitives/gLine.cpp
	${ENGINE_DIR}/graphics/primitives/gCircle.cpp
	${ENGINE_DIR}/graphics/primitives/gRectangle.cpp
	${ENGINE_DIR}/graphics/primitives/gPlane.cpp
	${ENGINE_DIR}/graphics/primitives/gBox.cpp
	${ENGINE_DIR}/graphics/primitives/gSphere.cpp
	${ENGINE_DIR}/graphics/gVbo.cpp
	${ENGINE_DIR}/graphics/gFbo.cpp
	${ENGINE_DIR}/graphics/gRay.cpp
	${ENGINE_DIR}/graphics/gBoundingBox.cpp
	${ENGINE_DIR}/graphics/gRect.cpp
	${ENGINE_DIR}/ui/gGUITextbox.cpp
	${ENGINE_DIR}/ui/gGUIText.cpp
	${ENGINE_DIR}/ui/gGUIDivider.cpp
	${ENGINE_DIR}/ui/gGUIToolbarButton.cpp
	${ENGINE_DIR}/ui/gGUIButton.cpp
	${ENGINE_DIR}/ui/gGUISizer.cpp
	${ENGINE_DIR}/ui/gGUIToolbar.cpp
	${ENGINE_DIR}/ui/gGUIMenubar.cpp
	${ENGINE_DIR}/ui/gGUIPanel.cpp
	${ENGINE_DIR}/ui/gGUIFrame.cpp
	${ENGINE_DIR}/ui/gGUIControl.cpp
	${ENGINE_DIR}/ui/gGUIContainer.cpp
	${ENGINE_DIR}/ui/gGUIForm.cpp
	${ENGINE_DIR}/ui/gGUIResources.cpp
	${ENGINE_DIR}/base/gBaseGUIObject.cpp
	${ENGINE_DIR}/core/gGUIManager.cpp
	${ENGINE_DIR}/core/gAppObject.cpp
	${ENGINE_DIR}/base/gBasePlugin.cpp
	${ENGINE_DIR}/media/gBaseSound.cpp
	${ENGINE_DIR}/media/gFmodSound.cpp
	${ENGINE_DIR}/base/gBaseApp.cpp
	${ENGINE_DIR}/base/gBaseCanvas.cpp
	${ENGINE_DIR}/core/gBaseWindow.cpp
	${ENGINE_DIR}/core/gCanvasManager.cpp
	${ENGINE_DIR}/core/gAppManager.cpp
	${ENGINE_DIR}/core/gPostProcessManager.cpp
	${ENGINE_DIR}/base/gBasePostProcess.cpp
	${ENGINE_DIR}/graphics/posteffects/gInversion.cpp
	${ENGINE_DIR}/graphics/posteffects/gGrayscale.cpp
	${ENGINE_DIR}/graphics/posteffects/gKernelEffect.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gBlur.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gSharpen.cpp
	${ENGINE_DIR}/graphics/posteffects/kerneleffects/gEdgeDetection.cpp	
)

if(NOT ANDROID)
	list(APPEND SRC_FILES
		${ENGINE_DIR}/core/backward.cpp
		${ENGINE_DIR}/core/gGLFWWindow.cpp
	)
endif(NOT ANDROID)

add_library(GlistEngine SHARED
	${SRC_FILES}
)


########## INCLUDE DIRECTORIES ##########
set(INC_DIRS
	${LIBS_DIR}/stb/include
	${LIBS_DIR}/fmod/include
	${LIBS_DIR}/utf8/include
	${LIBS_DIR}/ghc/include
    ${ENGINE_DIR}/ui
    ${ENGINE_DIR}/utils
    ${ENGINE_DIR}/types
    ${ENGINE_DIR}/media
    ${ENGINE_DIR}/graphics
    ${ENGINE_DIR}/graphics/primitives
    ${ENGINE_DIR}/graphics/posteffects
    ${ENGINE_DIR}/graphics/posteffects/kerneleffects
    ${ENGINE_DIR}/base
    ${ENGINE_DIR}/core
	${PLUGIN_INCLUDES}
#	${CMAKE_BINARY_DIR}/generated
)


if(WIN32)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include
		${MINGW64_DIR}/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++
		${MINGW64_DIR}/include/freetype2
		${MINGW64_DIR}/include/ffmpeg
		${MINGW64_DIR}/include/sqlite
		${MINGW64_DIR}/include/curl
	)
elseif(LINUX)
	list(APPEND INC_DIRS
		${MINGW64_DIR}/include
		${MINGW64_DIR}/include/freetype2
		${MINGW64_DIR}/include/sqlite
		# TODO: FFMPEG
		
		/usr/include/freetype2
		/usr/include
		/usr/local/lib
#		${MINGW64_DIR}/lib
	)
elseif(APPLE)
  	list(APPEND INC_DIRS
  		/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers
#  		/usr/local/Cellar/freetype/2.10.4/include
#  		/usr/local/Cellar/freetype/2.10.4/include/freetype2
#  		/usr/local/Cellar/glew/2.2.0_1/include
#  		/usr/local/Cellar/glm/0.9.9.8/include
#  		/usr/local/Cellar/assimp/5.0.1/include
#  		/usr/local/Cellar/curl/7.80.0_1/include
  		/usr/local/include
  		/usr/local/include/freetype2
		${MINGW64_DIR}/include/sqlite
		# TODO: FFMPEG
  	)
elseif(ANDROID)
	list(APPEND INC_DIRS
		${ANDROID_DIR}/include
		${ANDROID_DIR}/include/freetype2
	)
endif(WIN32)


include_directories(${INC_DIRS})

#message(WARNING "PLUGIN_LIBDIRS: ${PLUGIN_LIBDIRS}")
FOREACH(PLIBDIR ${PLUGIN_LIBDIRS})
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${PLIBDIR}")
ENDFOREACH()


########## FIND LIBRARIES ##########
if(WIN32)
	link_directories(
			${MINGW64_DIR}/bin
			${MINGW64_DIR}/x86_64-w64-mingw32/lib
	#		${LIBS_DIR}/fmod/lib
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
	
	add_library(glfw SHARED IMPORTED)
	set_target_properties(glfw PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libglfw.so)

	add_library(fmod SHARED IMPORTED)
	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libfmod.so)
	
	add_library(fmodL SHARED IMPORTED)
	set_target_properties(fmodL PROPERTIES IMPORTED_LOCATION
			${MINGW64_DIR}/lib/libfmodL.so)

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
	find_library(Lib_freetype freetype HINTS /usr/local/lib)
#	find_library(Lib_freetype freetype HINTS /usr/local/Cellar/freetype/2.10.4/lib)
	execute_process(COMMAND 
	    pkg-config --cflags freetype
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_1)
	add_definitions(${CL_VAR_1})

	find_library(Lib_assimp assimp HINTS /usr/local/lib)
#	find_library(Lib_assimp assimp HINTS /usr/local/Cellar/assimp/5.0.1/lib)
	execute_process(COMMAND 
	    pkg-config --cflags assimp
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_2)
	add_definitions(${CL_VAR_2})

	find_library(Lib_fmodL fmodL HINTS ${MINGW64_DIR}/lib)
	execute_process(COMMAND 
	    pkg-config --cflags fmod
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_3)
	add_definitions(${CL_VAR_3})

	find_library(Lib_fmod fmod HINTS ${MINGW64_DIR}/lib)
	execute_process(COMMAND 
	    pkg-config --cflags fmod
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_4)
	add_definitions(${CL_VAR_4})

	find_library(Lib_glfw glfw HINTS /usr/local/lib)
#	find_library(Lib_glfw glfw HINTS /usr/local/Cellar/glfw/3.3.3/lib)
	execute_process(COMMAND 
	    pkg-config --cflags glfw
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_5)
	add_definitions(${CL_VAR_5})

	find_library(Lib_GL GL HINTS /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries)
	execute_process(COMMAND 
	    pkg-config --cflags GL
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_6)
	add_definitions(${CL_VAR_6})

	find_library(Lib_GLEW GLEW HINTS /usr/local/lib)
#	find_library(Lib_GLEW GLEW HINTS /usr/local/Cellar/glew/2.2.0_1/lib)
	execute_process(COMMAND 
	    pkg-config --cflags GLEW
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_7)
	add_definitions(${CL_VAR_7})
	
	find_library(Lib_curl curl HINTS /usr/local/lib)
#	find_library(Lib_curl curl HINTS /usr/local/Cellar/curl/7.80.0_1/lib)
	execute_process(COMMAND 
	    pkg-config --cflags curl
	    OUTPUT_VARIABLE
	    CL_TMP_VAR
	    OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(STRIP "${CL_TMP_VAR}" CL_VAR_8)
	add_definitions(${CL_VAR_8})
	
elseif(ANDROID)
	find_library( # Sets the name of the path variable.
	              log-lib
	
	              # Specifies the name of the NDK library that
	              # you want CMake to locate.
	              log )
	
	
	add_library(fmod SHARED IMPORTED)
	set_target_properties(fmod PROPERTIES IMPORTED_LOCATION
			${ANDROID_DIR}/libs/fmod/${ANDROID_ABI}/libfmod.so)
	
	add_library(fmodL SHARED IMPORTED)
	set_target_properties(fmodL PROPERTIES IMPORTED_LOCATION
			${ANDROID_DIR}/libs/fmod/${ANDROID_ABI}/libfmodL.so)
	
	add_library(assimp STATIC IMPORTED)
	set_target_properties(assimp PROPERTIES IMPORTED_LOCATION
			${ANDROID_DIR}/libs/assimp/${ANDROID_ABI}/libassimp.a)
	
	add_library(libpng STATIC IMPORTED)
	set_target_properties(libpng PROPERTIES IMPORTED_LOCATION
			${ANDROID_DIR}/libs/libpng/${ANDROID_ABI}/libpng.a)
	
	add_library(freetype STATIC IMPORTED)
	set_target_properties(freetype PROPERTIES IMPORTED_LOCATION
			${ANDROID_DIR}/libs/freetype/${ANDROID_ABI}/libfreetype.a)
	
endif (WIN32)


########## LINK LIBRARIES ##########
set(LINK_LIBS
	${PLUGIN_LINKLIBS}
)

if(WIN32)
	list(APPEND LINK_LIBS
		fmodL
		fmod
		assimp
		freetype
		curl
		glfw3
		opengl32
		glew32
		dbghelp
		msvcr100
		psapi
#		swscale-5
#		swresample-3
#		avutil-56
#		avformat-58
#		avfilter-7
#		avdevice-58
#		avcodec-58
	)
elseif(LINUX)
	list(APPEND LINK_LIBS
		fmodL
		fmod
		assimp
		freetype
		libpng
		curl
		glfw
		GL
		GLEW
		dl
	# TODO: FFMPEG
	)
elseif(APPLE)
	list(APPEND LINK_LIBS
		${Lib_fmodL}
		${Lib_fmod}
		${Lib_assimp}
		${Lib_freetype}
		${Lib_curl}
		${Lib_glfw}
		${Lib_GL}
		${Lib_GLEW}
	# TODO: FFMPEG
	)
elseif(ANDROID)
	list(APPEND LINK_LIBS
		fmodL
		fmod
		assimp
		freetype
		libpng
		android
		log
		GLESv3
	)
endif(WIN32)

# add lib dependencies
target_link_libraries(GlistEngine
	${LINK_LIBS}
)
