project(DX12Renderer)

set(INC_FILES
	"Tutorial2.h"
)

set(SRC_FILES
	"main.cpp"
	"Tutorial2.cpp"
) 

add_executable(Tutorial2 ${SRC_FILES} ${INC_FILES})

# Include directories specific to this target
target_include_directories(Tutorial2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Tutorial2 DX12Lib)