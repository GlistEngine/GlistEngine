cmake_minimum_required (VERSION 3.10.2)


# Set project name
set(APP_NAME gisLight)

# List the plugins that will be used in this project
set(PLUGINS 
	gipEmpty
)



# DO NOT CHANGE BELOW
##################################################
project (${APP_NAME})

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

set(APP_DIR ${CMAKE_SOURCE_DIR})
set(MAIN_DIR ${APP_DIR}/../..)
set(TOP_DIR ${MAIN_DIR}/..)
set(ENGINE_DIR ${MAIN_DIR}/engine)
set(PLUGINS_DIR ${MAIN_DIR}/plugins)
set(LIBS_DIR ${MAIN_DIR}/libs)
set(MINGW64_DIR ${TOP_DIR}/zbin/glistzbin-win64/mingw64)


include(${ENGINE_DIR}/CMakeLists.txt)


add_executable(${APP_NAME} 
				${APP_DIR}/src/main.cpp
				${APP_DIR}/src/gApp.cpp
				${APP_DIR}/src/GameCanvas.cpp)


include_directories(${APP_NAME} PRIVATE
                        ${APP_DIR}/src)
                        
# add lib dependencies
target_link_libraries(${APP_NAME}
                      GlistEngine)
				
