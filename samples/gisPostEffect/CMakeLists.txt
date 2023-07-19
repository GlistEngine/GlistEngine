cmake_minimum_required (VERSION 3.10.2)


########## PROJECT NAME ##########
set(APP_NAME GlistApp)


########## USED PLUGINS LIST ##########
set(PLUGINS)


# MAC devs, please check OpenSSL versions after installing them via Homebrew,
# and write their version codes here
# M1 and M2 folder to check OpenSSL version code: /opt/homebrew/Cellar/openssl@1.1
# x86_64 folder to check OpenSSL version code:    /usr/local/Cellar/openssl@1.1
if(APPLE)
	set(OPENSSL_VER_SHORT "1.1")
	set(OPENSSL_VER_FULL "1.1.1t")
	
	# M1 and M2 devs should also check LLVM version in /opt/homebrew/Cellar/llvm
	# and write its version below
	set(LLVM_VER "16.0.0")
endif()


##################################################
# DO NOT CHANGE BELOW
##################################################

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

if(APPLE)
if(NOT ${DEVICEARCHITECTURE} STREQUAL "arm64")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -march=x86-64")
	set(SYS_NAME "macos-x86_64")
	set(SYS_PATH "/usr/local")
	set(BREW_PATH "${SYS_PATH}/Cellar")
	set(SYS_INC_PATH "${SYS_PATH}/include")
	set(CMAKE_C_COMPILER "/usr/local/bin/gcc-12")
	set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")
else()
	set(SYS_NAME "macos")
	set(SYS_PATH "/opt/homebrew")
	set(BREW_PATH "${SYS_PATH}/Cellar")
	set(SYS_INC_PATH "${SYS_PATH}/include")
	set(CMAKE_C_COMPILER "${BREW_PATH}/llvm/${LLVM_VER}/bin/clang")
	set(CMAKE_CXX_COMPILER "${BREW_PATH}/llvm/${LLVM_VER}/bin/clang++")
endif()
endif(APPLE)


########## PATHS AND VARIABLES ##########
set(APP_DIR ${CMAKE_SOURCE_DIR})
project (${APP_NAME})

set(TOP_DIR ${APP_DIR}/../..)
set(MAIN_DIR ${TOP_DIR}/GlistEngine)
set(ENGINE_DIR ${MAIN_DIR}/engine)

include(${ENGINE_DIR}/CMakeLists.txt)

########## SOURCE FILES AND INCLUDE DIRECTORIES ##########
add_executable(${APP_NAME} 
				${APP_DIR}/src/main.cpp
				${APP_DIR}/src/gApp.cpp
				${APP_DIR}/src/GameCanvas.cpp)


include_directories(${APP_NAME} PRIVATE
                        ${APP_DIR}/src)

########## LINKING ##########
# add lib dependencies
target_link_libraries(${APP_NAME}
                      GlistEngine)
				
