cmake_minimum_required(VERSION 3.13)

set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "")
project(VulkanLearn VERSION 1.0
	DESCRIPTION "learning Vulkan"
	LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

find_program(CCACHE ccache)
if(CCACHE)
	set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

# link this 'library' to set c++ standard / compile time options
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)


if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
	option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
	if(ENABLE_BUILD_WITH_TIME_TRACE)
		add_compile_definitions(project_warnings INTERFACE -ftime-trace)
	endif()
endif()

# link this 'library' to use standard warnings in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer optoins if supported by compilers 
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_warnings)

#enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

include(FindVulkan)
include(FindOpenMP)

# find required programs
if(Vulkan_FOUND) 
	MESSAGE("Found Vulkan SDK.")
else()
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)

endif()

if(OpenMP_CXX_FOUND) 
	MESSAGE("Found OpenMP.")
else()
	MESSAGE("Could not find OpenMP.  Verify that it is installed.")
	return()
endif()

#################################################################################################################################
# options
#################################################################################################################################

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_CLANG_TIDY "Enable testing with clang-tidy" ON)
option(ENABLE_CPPCHECK "Enable testing with cppcheck" OFF)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
	# sets a global PCH parameter, each project will build its own PCH though
	target_precompile_headers(
		project_options
		INTERFACE
		<vector>
		<string>
		<map>
		<utility>
		)
endif()

# set up extra conan dependencies 
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

list(APPEND EXTRA_LIBS 
	project_options 
	project_warnings
	OpenMP::OpenMP_CXX
	Vulkan::Vulkan
	CONAN_PKG::spdlog
	CONAN_PKG::glm
	CONAN_PKG::glfw
	CONAN_PKG::stb
	CONAN_PKG::tinyobjloader
	CONAN_PKG::imgui
	CONAN_PKG::sdl2
	)

include(cmake/ConanConfig.cmake)
run_conan()



#################################################################################################################################
# project setup
#################################################################################################################################

add_subdirectory(src)

# TODO 
# add step where shaders are compiled and put into the ./shader/bin dir

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/shader)

# step for copying shader binaries to output dir
file(COPY 
	${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/
	DESTINATION
	${CMAKE_BINARY_DIR}/bin/shader/
)

# step for copying textures to output dir
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/res/tex)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/res/model)

file(COPY
	${CMAKE_CURRENT_SOURCE_DIR}/res/tex
	DESTINATION
	${CMAKE_BINARY_DIR}/bin/res
)

file(COPY
	${CMAKE_CURRENT_SOURCE_DIR}/res/model
	DESTINATION
	${CMAKE_BINARY_DIR}/bin/res
)

####################################################################################


