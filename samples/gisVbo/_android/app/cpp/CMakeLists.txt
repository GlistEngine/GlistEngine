cmake_minimum_required (VERSION 3.10.2)

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

########## PROJECT NAME COMES FROM MAIN CMAKE WHICH IS UPPER FOLDER##########
if(NOT DEFINED APP_NAME)
    set(APP_NAME GlistApp)
endif()

########## USED PLUGINS LIST ##########
set(PLUGINS)

##################################################
# DO NOT CHANGE BELOW
##################################################

########## PATHS AND VARIABLES ##########
project (${APP_NAME})


set(TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../..")
set(MAIN_DIR ${TOP_DIR}/GlistEngine)
set(ENGINE_DIR ${MAIN_DIR}/engine)

list(APPEND PLUGINS gipAndroid)

include(${ENGINE_DIR}/CMakeLists.txt)

if(NOT DEFINED GLIST_APP_REL_PATH)
    set(GLIST_APP_REL_PATH src/GlistApp)
endif()

include_directories(${GLIST_APP_REL_PATH}/src)
add_subdirectory(${GLIST_APP_REL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/GlistApp)

if(TARGET ${APP_NAME})
    target_link_libraries(${APP_NAME} PRIVATE atomic)
endif()
