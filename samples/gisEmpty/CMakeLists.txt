cmake_minimum_required (VERSION 3.10.2)

# Set project name
set(APP_NAME gisEmpty)


# Do not change below
set(APP_DIR ${CMAKE_SOURCE_DIR})
project (${APP_NAME})

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

set(MAIN_DIR ${APP_DIR}/../..)
set(TOP_DIR ${MAIN_DIR}/..)
set(ENGINE_DIR ${MAIN_DIR}/engine)

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
				
