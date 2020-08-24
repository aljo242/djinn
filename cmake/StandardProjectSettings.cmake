# set a default build type if none was specified 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS
		"Settings build type to 'Debug' as none was specified"
		)
	set(CMAKE_BUILD_TYPE
		Debug
		CACHE STRING "Choose the type of build: " FORCE
		)
	# set the possible values fo the build type for cmake-gui or ccmake
	set_property(
		CACHE CMAKE_BUILD_TYPE
		PROPERTY STRINGS
		"Debug"
		"Release"
		"MinSizeRel"
		"RelWithDebInfo"
		)
endif()

# generate compile_commands.json to make it easier to work with clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_IPO "Enable Interprocedural Optimization (Link Time Optimization)" OFF)
if(ENABLE_IPO)
	include(CheckIPOSupported)
	check_ipo_supported(
		RESULT
		result 
		OUTPUT
		output 
		)
	if(result)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
	else()
		message(SEND_ERROR
			"IPO is not supported: ${output}"
			)
	endif()
endif()