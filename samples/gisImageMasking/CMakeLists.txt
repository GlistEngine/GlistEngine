cmake_minimum_required (VERSION 3.10.2)

########## PROJECT NAME ##########
set(APP_NAME GlistApp)


########## USED PLUGINS LIST ##########
set(PLUGINS)


########## CPP VERSION ##########
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


########## ENGINE BUILDING (SHARED / STATIC) (CHANGE TO STATIC IF REALLY NEEDED) ##########
set(BUILD_TYPE SHARED)



################################################################
# DO NOT CHANGE ANYTHING BELOW UNLESS YOU KNOW WHAT YOU DO
################################################################

########## CMAKE THING ##########
if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
	set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()


########## WINDOWS COMPILER PATH ##########
# Clang compiler definition for Windows. Do not change this value for other OS.
# For UNIX like systems, configure the compiler via OS commands.
if(WIN32)
	set(CMAKE_C_COMPILER "${CMAKE_SOURCE_DIR}/../../zbin/glistzbin-win64/clang64/bin/clang.exe")
	set(CMAKE_CXX_COMPILER "${CMAKE_SOURCE_DIR}/../../zbin/glistzbin-win64/clang64/bin/clang++.exe")
endif(WIN32)


########## MACOS ARCHITECTURE ##########
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE DEVICEARCHITECTURE )
message( STATUS "Architecture: ${DEVICEARCHITECTURE}" )

set(GLIST_CXX_FLAGS_RELEASE)
set(GLIST_CXX_FLAGS_DEBUG)

if(APPLE)
	if(NOT ${DEVICEARCHITECTURE} STREQUAL "arm64")
		set(GLIST_CXX_FLAGS_RELEASE "-march=x86-64")
		set(GLIST_CXX_FLAGS_DEBUG "-march=x86-64")
 		set(SYS_NAME "macos-x86_64")
		set(SYS_PATH "/usr/local")
	else()
		set(SYS_NAME "macos")
		set(SYS_PATH "/opt/homebrew")
	endif()
	set(BREW_PATH "${SYS_PATH}/opt")
	set(SYS_INC_PATH "${SYS_PATH}/include")
	set(ENGINE_USE_SYMLINKS ON)
endif(APPLE)

if(IOS)
	if(${GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER} STREQUAL "")
		message(FATAL_ERROR "The variable GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER was not supplied!")
	endif()
    
	if(${GLIST_IOS_DEVELOPMENT_TEAM} STREQUAL "")
		message(FATAL_ERROR "The variable GLIST_IOS_DEVELOPMENT_TEAM was not supplied!")
	endif()
    
		if(${GLIST_IOS_PRODUCT_NAME} STREQUAL "")
		message(FATAL_ERROR "The variable GLIST_IOS_PRODUCT_NAME was not supplied!")
	endif()

	set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER})
	set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_NAME ${GLIST_IOS_PRODUCT_NAME})
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "AUTOMATIC")
	set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${GLIST_IOS_DEVELOPMENT_TEAM})

	add_compile_definitions(GLES_SILENCE_DEPRECATION)
endif()


########## PATHS AND VARIABLES ##########
set(APP_DIR ${CMAKE_SOURCE_DIR})
project (${APP_NAME})

enable_language(CXX)
enable_language(C)
if(IOS)
	enable_language(OBJCXX)
endif()

set(TOP_DIR ${APP_DIR}/../..)
set(MAIN_DIR ${TOP_DIR}/GlistEngine)
set(ENGINE_DIR ${MAIN_DIR}/engine)

if(APPLE)
	add_subdirectory(${ENGINE_DIR} ./_build)
else()
	include(${ENGINE_DIR}/CMakeLists.txt)
endif()

########## SOURCE FILES AND INCLUDE DIRECTORIES ##########

set(GlistApp_SOURCES
		${APP_DIR}/src/main.cpp
		${APP_DIR}/src/gApp.cpp
		${APP_DIR}/src/gCanvas.cpp
)

set(GlistApp_HEADERS
		${APP_DIR}/src/gApp.h
		${APP_DIR}/src/gCanvas.h
)

add_executable(${APP_NAME}
		${GlistApp_SOURCES}
		${GlistApp_HEADERS}
)

######### APPLE AND IOS SPECIFIC SETTINGS #########

if(IOS)
	if(PLUGINS MATCHES "^.*gipIOS.*$")
		include(${TOP_DIR}/glistplugins/gipIOS/CMakeLists.txt)
	endif()

	set_source_files_properties(macos/GlistApp.storyboard PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	target_sources(${APP_NAME}
		PRIVATE 
			${APP_DIR}/macos/GlistApp.storyboard
			${IOS_PLUGIN_HEADERS}
			${IOS_PLUGIN_SRCS}
	)

	target_include_directories(${APP_NAME} PRIVATE ${IOS_PLUGIN_INCLUDES})
	target_include_directories(GlistEngine PRIVATE ${IOS_PLUGIN_INCLUDES})
	target_link_libraries(${APP_NAME} PRIVATE ${IOS_PLUGIN_LINKLIBS})

	set_target_properties(${APP_NAME}
		PROPERTIES
			XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${GLIST_IOS_PRODUCT_BUNDLE_IDENTIFIER}
			XCODE_ATTRIBUTE_PRODUCT_NAME 			  ${GLIST_IOS_PRODUCT_NAME}
			XCODE_ATTRIBUTE_CODE_SIGN_STYLE           "AUTOMATIC"
			XCODE_ATTRIBUTE_DEVELOPMENT_TEAM          ${GLIST_IOS_DEVELOPMENT_TEAM}
			XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY    "1,2"
	)

	#Put assets to the Resources folder of the bundle
	file(GLOB_RECURSE RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

	target_sources(${APP_NAME} PRIVATE ${RESOURCES})

	foreach (FILE ${RESOURCES})
		file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets" ${FILE})
		get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
		set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")
		source_group("Resources/${NEW_FILE_PATH}" FILES "${FILE}")
	endforeach ()

	set_property(
		TARGET ${APP_NAME}
		PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist
	)
endif()

if(APPLE)
	target_include_directories(
	    ${APP_NAME}
	        PRIVATE
	            ${APP_DIR}/src
	)
else()
	include_directories(${APP_NAME} PRIVATE
                        ${APP_DIR}/src)
endif()

########## LINKING ##########
# add lib dependencies
target_link_libraries(${APP_NAME} PRIVATE GlistEngine)

install(
    DIRECTORY assets
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
)
