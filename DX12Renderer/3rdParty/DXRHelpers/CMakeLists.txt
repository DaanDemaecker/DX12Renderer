
project(DX12Renderer)

set(INC_FILES
	"DXRHelper.h"
	"nv_helpers_dx12/BottomLevelASGenerator.h"
	"nv_helpers_dx12/RaytracingPipelineGenerator.h"
	"nv_helpers_dx12/RootSignatureGenerator.h"
	"nv_helpers_dx12/ShaderBindingTableGenerator.h"
	"nv_helpers_dx12/TopLevelASGenerator.h"
)

set(SRC_FILES
	
	"nv_helpers_dx12/BottomLevelASGenerator.cpp"
	"nv_helpers_dx12/RaytracingPipelineGenerator.cpp"
	"nv_helpers_dx12/RootSignatureGenerator.cpp"
	"nv_helpers_dx12/ShaderBindingTableGenerator.cpp"
	"nv_helpers_dx12/TopLevelASGenerator.cpp"
)

add_library(DXRHelpers ${SRC_FILES} ${INC_FILES})

target_link_libraries(DX12Lib PUBLIC DXRHelpers)