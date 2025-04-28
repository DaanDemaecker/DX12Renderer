project(DX12Renderer)

set(INC_FILES

)

set(SRC_FILES
	"main.cpp"
) 

add_executable(RayTracer ${SRC_FILES} ${INC_FILES})

# Include directories specific to this target
target_include_directories(RayTracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RayTracer DX12Lib)