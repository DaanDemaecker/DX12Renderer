project(DX12Renderer)

set(INC_FILES
 "Tutorial3.h")

set(SRC_FILES
	"main.cpp"
 "Tutorial3.cpp") 

add_executable(Tutorial3 ${SRC_FILES} ${INC_FILES})

# Include directories specific to this target
target_include_directories(Tutorial3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Tutorial3 DX12Lib)