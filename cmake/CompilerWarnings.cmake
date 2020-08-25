# All taken from Jason Turner (https://github.com/lefticus) 
# retyped by me https://github.com/aljo242 for learning

# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Avai lable.md

function(set_project_warnings project_name)
	option(
		WARNINGS_AS_ERRORS 
		"Treat compiler warnings as errors" 
		OFF
		)

	set(MSVC_WARNINGS

		/W4   					# baseline warnings level
		/w14242					# 'identifier' possible loss of data conversion warnings 
		/w14254					# 'operator' possible loss of data conversion warnings  
		/w14263					# 'function' member function does not override any base virtual function
		/w14265					# 'classname' class has virtual functions but not virtual dtor
		/w14286					# 'operator' unsigned/negative constant mismatch
		/we4289					# loop control variable declared in the for-loop is used outside for-loop scoope
		/w14296					# expression is always TRUE
		/w14311					# pointer truncation 
		/w14545					# expression before comma missing argument list
		/w14546					# function call before comma missing arg list
		/w14547					# operator before comma has no effect
		/w14549					# operator before comma has no effect
		/w14555					# expression has no effect
		/w14619					# pragma warning: warning number does not exist
		/w14640					# enable warning on thread un-safe static member identifier
		/w14286  				# type conversion is sign-extended
		/w14905					# wide string literal cast to 'LPWSTR'
		/w14906					# string literal cast to 'LPWSTR'
		/w14928					# illegal copy-initialization 
		/permissive				# standards conformance
		/wd26812
		)

	set(CLANG_WARNINGS

		-Wall
		-Wextra 				# reasonable and standards
		-Wshadow				# warn for variable declaration shadowing
		-Wnon-virtual-dtor		#warn if class w v functions has no v dtor
		-Wold-style-cast		# no c-style casts
		-Wcast-align			# warn for potential performance problem casts
		-Wunused				# anything unused
		-Woverloaded-virtual	# overload (not override) virtual fxn
		-Wpedantic				# warn for non-standard C++
		-Wconversion			# type conversions that may lose data
		-Wsign-conversion		# 
		-Wnull-dereference		# 
		-Wdouble-promotion		# float implicitly converted to double
		-Wformat=2				# security issues around output formating (ie printf)
		)

	if(WARNINGS_AS_ERRORS)
		set(CLANG_WARNINGS 
			${CLANG_WARNINGS} 
			-Werror
			)

		set(MSVC_WARNINGS 
			${MSVC_WARNINGS} 
			/WX
			)
	endif()

	set(GCC_WARNINGS
		${CLANG_WARNINGS}
		-Wmisleading-indentation# indentation implies blocks where blocks do not exist
		-Wduplicated-cond		# if/else chain has duplicated conditions
		-Wduplicated-branches	# if/else branches have duplicated code 
		-Wlogical-op			# warn about logical operations being use where bitwise were probably wanted
		-Wuseless-cast			# casting a type to the same type
		)

	if(MSVC)
		set(PROJECT_WARNINGS ${MSVC_WARNINGS})
	elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
		set(PROJECT_WARNINGS ${CLANG_WARNINGS})
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(PROJECT_WARNINGS ${GCC_WARNINGS})
	else()
		message(AUTHOR_WARNING
			"No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler")
	endif()

	target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})

endfunction()