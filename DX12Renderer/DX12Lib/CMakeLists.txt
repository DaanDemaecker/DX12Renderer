project(DX12Renderer)

set(INC_FILES
    "src/Application/Application.h"
    "src/Application/CommandQueue.h"
    "src/Application/Events.h"
    "src/Application/HighResClock.h"
    "src/Application/KeyCodes.h"
    "src/Application/Singleton.h"
    "src/Application/UploadBuffer.h"
    "src/Application/Window.h"
    "src/Games/Game.h"
    
    "src/Helpers/Defines.h"
    "src/Helpers/DirectXHelpers.h"
    "src/Helpers/Helpers.h"
    "src/Includes/DirectXIncludes.h"
 "src/Application/CommandList.h"
 "src/Application/DescriptorAllocator/DescriptorAllocator.h"
 "src/Application/DescriptorAllocator/DescriptorAllocatorPage.h"
 "src/Application/DescriptorAllocator/DescriptorAllocation.h"
 "src/Application/Resources/Resource.h"
 "src/Application/DynamicDescriptorHeap.h"
 "src/Application/RootSignature.h"
 "src/Application/Resources/ResourceStateTracker.h"
 "src/Application/DataTypes/Mesh.h"
 "src/Includes/GlmIncludes.h"
 "src/Application/DataTypes/Structs.h"
 "src/Application/Buffers/Buffer.h"
 "src/Application/Buffers/IndexBuffer.h" "src/Application/Buffers/VertexBuffer.h" "src/Includes/DXRHelpersIncludes.h")

set(SRC_FILES
"src/Application/Window.cpp"
"src/Application/Application.cpp"
"src/Application/CommandQueue.cpp"
"src/Games/Game.cpp"

"src/Application/HighResClock.h"
"src/Application/HighResClock.cpp"
"src/Application/UploadBuffer.cpp"
"src/Application/CommandList.cpp"
 "src/Application/DescriptorAllocator/DescriptorAllocator.cpp"
 "src/Application/DescriptorAllocator/DescriptorAllocatorPage.cpp"
 "src/Application/DescriptorAllocator/DescriptorAllocation.cpp"
 "src/Application/Resources/Resource.cpp"
 "src/Application/DynamicDescriptorHeap.cpp"
 "src/Application/RootSignature.cpp"
 "src/Application/Resources/ResourceStateTracker.cpp"
 "src/Application/DataTypes/Mesh.cpp"
 "src/Application/Buffers/Buffer.cpp"
 "src/Application/Buffers/IndexBuffer.cpp" "src/Application/Buffers/VertexBuffer.cpp") 


add_library(DX12Lib ${SRC_FILES} ${INC_FILES})

# Include directories specific to this target
target_include_directories(DX12Lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(DX12Lib PUBLIC user32.lib kernel32.lib)