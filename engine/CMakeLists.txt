cmake_minimum_required (VERSION 3.10.2)


# Configure compiler and linker as you need
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++14 -DNDEBUG -DENABLE_BOOST_WORKAROUND=ON -Wuninitialized -Wmultichar -ffunction-sections -fdata-sections -fexceptions -frtti -fpermissive -fPIC -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -std=c++14 -g -DDEBUG -DENABLE_BOOST_WORKAROUND=ON -Wuninitialized -Wmultichar -ffunction-sections -fdata-sections -fexceptions -frtti -fpermissive -fPIC -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -v -Wl,--no-as-needed -Wl,--gc-sections -Wl,--unresolved-symbols=report-all")


# DO NOT CHANGE BELOW
##################################################
if(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -march=x86-64")
endif(WIN32)

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
    set(PLUGINS_DIR ${MAIN_DIR}/plugins)
endif()
if(NOT DEFINED MINGW64_DIR)
	set(MINGW64_DIR ${TOP_DIR}/zbin/glistzbin-win64/mingw64)
endif()
if(NOT DEFINED ANDROID_DIR)
	set(ANDROID_DIR ${TOP_DIR}/zbin/android)
endif()


#configure_file( ${ENGINE_DIR}/gConfig.h.in ${CMAKE_BINARY_DIR}/generated/gConfig.h )
  
if(NOT DEFINED PLUGINS)
    set(PLUGINS)
endif()
set(PLUGIN_SRCS)
set(PLUGIN_INCLUDES)
set(PLUGIN_LINKDIRS)
set(PLUGIN_LINKLIBS)

foreach(pluginname ${PLUGINS})
	include(${PLUGINS_DIR}/${pluginname}/CMakeLists.txt)
endforeach()


if(WIN32 OR LINUX OR APPLE)
add_library(GlistEngine SHARED
			${PLUGIN_SRCS}
			${ENGINE_DIR}/core/backward.cpp
			${ENGINE_DIR}/types/gColor.cpp
			${ENGINE_DIR}/utils/gUtils.cpp
			${ENGINE_DIR}/graphics/gShader.cpp
			${ENGINE_DIR}/core/gObject.cpp
			${ENGINE_DIR}/core/gRenderer.cpp
			${ENGINE_DIR}/core/gRenderObject.cpp
			${ENGINE_DIR}/graphics/gTexture.cpp
			${ENGINE_DIR}/graphics/gImage.cpp
			${ENGINE_DIR}/graphics/gNode.cpp
			${ENGINE_DIR}/graphics/gLight.cpp
			${ENGINE_DIR}/graphics/gCamera.cpp
			${ENGINE_DIR}/graphics/gFont.cpp
			${ENGINE_DIR}/graphics/gMaterial.cpp
			${ENGINE_DIR}/graphics/gMesh.cpp
			${ENGINE_DIR}/graphics/gSkinnedMesh.cpp
			${ENGINE_DIR}/graphics/gModel.cpp
			${ENGINE_DIR}/graphics/primitives/gPlane.cpp
			${ENGINE_DIR}/graphics/primitives/gBox.cpp
			${ENGINE_DIR}/graphics/gVbo.cpp
			${ENGINE_DIR}/core/gAppObject.cpp
			${ENGINE_DIR}/base/gBasePlugin.cpp
			${ENGINE_DIR}/media/gBaseSound.cpp
			${ENGINE_DIR}/media/gFmodSound.cpp
			${ENGINE_DIR}/base/gBaseApp.cpp
			${ENGINE_DIR}/base/gBaseCanvas.cpp
			${ENGINE_DIR}/core/gBaseWindow.cpp
			${ENGINE_DIR}/core/gGLFWWindow.cpp
			${ENGINE_DIR}/core/gCanvasManager.cpp
			${ENGINE_DIR}/core/gAppManager.cpp
)
elseif(ANDROID)
add_library(GlistEngine SHARED
			${PLUGIN_SRCS}
			${ENGINE_DIR}/types/gColor.cpp
			${ENGINE_DIR}/utils/gUtils.cpp
			${ENGINE_DIR}/graphics/gShader.cpp
			${ENGINE_DIR}/core/gObject.cpp
			${ENGINE_DIR}/core/gRenderer.cpp
			${ENGINE_DIR}/core/gRenderObject.cpp
			${ENGINE_DIR}/graphics/gTexture.cpp
			${ENGINE_DIR}/graphics/gImage.cpp
			${ENGINE_DIR}/graphics/gNode.cpp
			${ENGINE_DIR}/graphics/gLight.cpp
			${ENGINE_DIR}/graphics/gCamera.cpp
			${ENGINE_DIR}/graphics/gFont.cpp
			${ENGINE_DIR}/graphics/gMaterial.cpp
			${ENGINE_DIR}/graphics/gMesh.cpp
			${ENGINE_DIR}/graphics/gSkinnedMesh.cpp
			${ENGINE_DIR}/graphics/gModel.cpp
			${ENGINE_DIR}/graphics/primitives/gPlane.cpp
			${ENGINE_DIR}/graphics/primitives/gBox.cpp
			${ENGINE_DIR}/graphics/gVbo.cpp
			${ENGINE_DIR}/core/gAppObject.cpp
			${ENGINE_DIR}/base/gBasePlugin.cpp
			${ENGINE_DIR}/media/gBaseSound.cpp
			${ENGINE_DIR}/media/gFmodSound.cpp
			${ENGINE_DIR}/base/gBaseApp.cpp
			${ENGINE_DIR}/base/gBaseCanvas.cpp
			${ENGINE_DIR}/core/gBaseWindow.cpp
//			${ENGINE_DIR}/core/gGLFWWindow.cpp
			${ENGINE_DIR}/core/gCanvasManager.cpp
			${ENGINE_DIR}/core/gAppManager.cpp
)
endif(WIN32 OR LINUX OR APPLE)


if(WIN32)
include_directories(GlistEngine PRIVATE
						${PLUGIN_INCLUDES}
						${MINGW64_DIR}/include
						${MINGW64_DIR}/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++
						${MINGW64_DIR}/include/freetype2
						${LIBS_DIR}/stb/include
						${LIBS_DIR}/fmod/include
						${LIBS_DIR}/utf8/include
                        ${ENGINE_DIR}/utils
                        ${ENGINE_DIR}/types
                        ${ENGINE_DIR}/media
                        ${ENGINE_DIR}/graphics
                        ${ENGINE_DIR}/graphics/primitives
                        ${ENGINE_DIR}/base
                        ${ENGINE_DIR}/core
#                        ${CMAKE_BINARY_DIR}/generated
)
elseif(ANDROID)
include_directories(GlistEngine PRIVATE
		${ANDROID_DIR}/include
		${ANDROID_DIR}/include/freetype2
						${LIBS_DIR}/stb/include
)
	message(WARNING "SYSTEM: ANDROID")
endif (WIN32)


if(WIN32)
#find_library(GL opengl32 HINTS C:/Windows/system32)
#find_library(MSVCR msvcr100 HINTS C:/Windows/system32)

link_directories(
		${MINGW64_DIR}/bin
		${MINGW64_DIR}/x86_64-w64-mingw32/lib
#		${LIBS_DIR}/fmod/lib
		${PLUGIN_LINKDIRS}
)
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



# add lib dependencies
if(WIN32)
target_link_libraries(GlistEngine
					  ${PLUGIN_LINKLIBS}
					  assimp
					  freetype
					  fmodL
					  fmod
					  glfw3
					  opengl32
                      glew32
                      dbghelp
                      msvcr100
                      psapi
                      )
elseif(ANDROID)
target_link_libraries(GlistEngine
					  ${PLUGIN_LINKLIBS}
						assimp
						fmod
						fmodL
						freetype
						libpng
					  android
					  log
					  GLESv3
#                       ${log-lib}
                      )
endif (WIN32)
