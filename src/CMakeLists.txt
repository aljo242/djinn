add_executable(main
	hello.cpp
	 "QueueFamilies.h" 
	 "ext_inc.h" "ShaderLoader.h")

target_link_libraries(main PUBLIC
		${EXTRA_LIBS}
	)

target_include_directories(main PUBLIC
	${PROJECT_BINARY_DIR}
	)