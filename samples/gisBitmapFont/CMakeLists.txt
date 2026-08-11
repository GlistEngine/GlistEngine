cmake_minimum_required(VERSION 3.10.2)

set(APP_NAME GlistApp)
set(PLUGINS)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_TYPE SHARED)
set(APP_DIR ${CMAKE_SOURCE_DIR})
get_filename_component(MAIN_DIR "${APP_DIR}/../.." ABSOLUTE)
set(ENGINE_DIR ${MAIN_DIR}/engine)

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
	set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

if(WIN32)
	set(CMAKE_C_COMPILER "${MAIN_DIR}/../zbin/glistzbin-win64/clang64/bin/clang.exe")
	set(CMAKE_CXX_COMPILER "${MAIN_DIR}/../zbin/glistzbin-win64/clang64/bin/clang++.exe")
endif()

project(${APP_NAME})

if(APPLE)
	add_subdirectory(${ENGINE_DIR} ./_build)
else()
	include(${ENGINE_DIR}/CMakeLists.txt)
endif()

add_executable(${APP_NAME}
	${APP_DIR}/src/main.cpp
	${APP_DIR}/src/gApp.cpp
	${APP_DIR}/src/gCanvas.cpp
	${APP_DIR}/src/gApp.h
	${APP_DIR}/src/gCanvas.h
)

target_include_directories(${APP_NAME} PRIVATE ${APP_DIR}/src)
target_link_libraries(${APP_NAME} PRIVATE GlistEngine)

install(DIRECTORY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
