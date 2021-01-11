add_executable(main
	
	 "QueueFamilies.h" 
	 "ext_inc.h" 
	 "ShaderLoader.h" 
	 "ShaderLoader.cpp"
      "gfxDebug.h" 
	  
	 "gfxDebug.cpp" 
	  
	 "main.cpp" "QueueFamilies.cpp"  "DebugMessenger.h"  "core/core.h"  "core/Context.h" "core/Context.cpp" "core/defs.h" "core/SwapChain.h" "core/SwapChain.cpp" "core/Image.h"  "core/Memory.h" "core/Memory.cpp" "core/RenderPass.h" "core/Image.cpp" "DjinnLib/Utils.h" "DjinnLib/Types.h" "core/Buffer.h" "core/Buffer.cpp" "core/Commands.h" "core/Commands.cpp" "core/GraphicsPipeline.h" "core/GraphicsPipeline.cpp" "core/Primitives.h"  "core/core.cpp" "core/RenderPass.cpp" "VulkanEngine.h" "VulkanEngine.cpp" "App.h" "App.cpp" "core/IO.h" "DjinnLib/Queue.h")

target_link_libraries(main PUBLIC
		${EXTRA_LIBS}
		
	)

target_include_directories(main PUBLIC
	${PROJECT_BINARY_DIR}
	)