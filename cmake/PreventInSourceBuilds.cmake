# function will prevent in-source builds
function(AssureOutOfSourceBuilds)
	# make sure the user doesn't play dirty wit symlinks
	get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
	get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

	#disallow in-src builds
	if("${srcdir}" STREQUAL "${bindir}")
		message("################################################")
		message("Warning: in-source builds are disabled")
		message("Please create a separate build directory and run cmake there")
		message("################################################")
		message(FATAL_ERROR
			"Quitting Configuration"
			)
	endif() 
endfunction()

assureoutofsourcebuilds()