add_executable(main
	hello.cpp
	 "QueueFamilies.h" 
	 "Warnings.h")

target_link_libraries(main PUBLIC
		${EXTRA_LIBS}
	)

target_include_directories(main PUBLIC
	${PROJECT_BINARY_DIR}
	)