add_executable(main
	hello.cpp
	 "QueueFamilies.h" 
	 "ext_inc.h" 
	 "ShaderLoader.h" 
	 "ShaderLoader.cpp"
	 "hello.h" "gfxDebug.h" 
	  
	 "gfxDebug.cpp" 
	  
	 "main.cpp" "QueueFamilies.cpp"  "DebugMessenger.h"  "core/core.h"  "core/Context.h" "core/Context.cpp" "core/defs.h" "core/SwapChain.h" "core/SwapChain.cpp" "core/Image.h"  "core/Memory.h" "core/Memory.cpp" "core/RenderPass.h" "core/Image.cpp")

target_link_libraries(main PUBLIC
		${EXTRA_LIBS}
	)

target_include_directories(main PUBLIC
	${PROJECT_BINARY_DIR}
	)