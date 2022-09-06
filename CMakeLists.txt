cmake_minimum_required (VERSION 3.10)
project(sdl_gamecontroller CXX)

message(STATUS "Build type ${CMAKE_BUILD_TYPE}")

# Add SDL2
find_package(SDL2 REQUIRED)
if(IS_DIRECTORY "${SDL2_INCLUDE_DIRS}")
 get_filename_component(SDL2_PARENT_DIR ${SDL2_INCLUDE_DIRS} DIRECTORY)
 message(STATUS "Found SDL2 includes in ${SDL2_INCLUDE_DIRS} in ${SDL2_PARENT_DIR}")
 include_directories(SYSTEM "${SDL2_PARENT_DIR}")
else()
 message(STATUS "SDL2 includes is probably multiple directories ${SDL2_INCLUDE_DIRS}")
 include_directories(SYSTEM "${SDL2_INCLUDE_DIRS}")
endif()

# Add Node
include_directories(SYSTEM ${CMAKE_JS_INC})
message(STATUS "Found Node in ${CMAKE_JS_INC}")

# Include node-addon-api wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
include_directories(SYSTEM ${NODE_ADDON_API_DIR})
add_definitions(-DNAPI_VERSION=3)

#
# Debugging Options
#
set (CMAKE_VERBOSE_MAKEFILE 0) # Use 1 for debugging, 0 for release

#
# Compiler flags
#
add_compile_options(-Wall -Wextra -pedantic -Werror)

#
# Project Search Paths
#
aux_source_directory(src project_source_files)

# Set library name
add_library(${PROJECT_NAME} SHARED ${project_source_files} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${CMAKE_JS_LIB})

# Require the right compiler
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED true)
