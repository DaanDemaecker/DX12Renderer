// RayTracingScene.cpp

// Header include
#include "RayTracingScene.h"

// File includes
#include "Application/Application.h"
#include "Includes/DirectXIncludes.h"
#include "Application/Window.h"
#include "Helpers/Helpers.h"
#include "Application/CommandList.h"

// Vertex data for a colored cube.
struct VertexPosColor2
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

static VertexPosColor2 g_Vertices[8] = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
};

static WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

DDM::RayTracingScene::RayTracingScene(const std::wstring& name, int width, int height, bool vSync)
	:Game(name, width, height, vSync),
	m_scissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
	m_viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
{
	DDM::Application::Get().QueryRaytracingSupport();

	m_renderTargets.resize(DDM::Application::Get().FrameCount());	

    m_device = Application::Get().GetDevice();
}

// Load the sample assets.
bool DDM::RayTracingScene::LoadContent()
{
	LoadAssets();

	Application::Get().QueryRaytracingSupport();



	return true;
}

void DDM::RayTracingScene::UnloadContent()
{
}

void DDM::RayTracingScene::OnRender(RenderEventArgs& e)
{

}

void DDM::RayTracingScene::LoadAssets()
{

    m_commandList = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->GetCommandList()->GetGraphicsCommandList();

    // Create an empty root signature.
    {
        // #DXR Extra: Perspective Camera
        // The root signature describes which data is accessed by the shader. The
        // camera matrices are held in a constant buffer, itself referenced the
        // heap. To do this we reference a range in the heap, and use that range as
        // the sole parameter of the shader. The camera buffer is associated in the
        // index 0, making it accessible in the shader in the b0 register.
        CD3DX12_ROOT_PARAMETER constantParameter;
        CD3DX12_DESCRIPTOR_RANGE range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        constantParameter.InitAsDescriptorTable(1, &range,
            D3D12_SHADER_VISIBILITY_ALL);

        // #DXR Extra - Refitting
        // Per-instance properties buffer
        CD3DX12_ROOT_PARAMETER matricesParameter;
        CD3DX12_DESCRIPTOR_RANGE matricesRange;
        matricesRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1 /* desc count*/,
            0 /*register*/, 0 /*space*/, 1 /*heap slot*/);
        matricesParameter.InitAsDescriptorTable(1, &matricesRange,
            D3D12_SHADER_VISIBILITY_ALL);

        // #DXR Extra - Refitting
        // Per-instance properties index for the current geometry
        CD3DX12_ROOT_PARAMETER indexParameter;
        indexParameter.InitAsConstants(1 /*value count*/, 1 /*register*/);

        // #DXR Extra - Refitting
        std::vector<CD3DX12_ROOT_PARAMETER> params = {
            constantParameter, matricesParameter, indexParameter };

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(
            static_cast<UINT>(params.size()), params.data(), 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(
            &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)));
    }


    // Create the vertex buffer.
    {

        const UINT vertexBufferSize = sizeof(g_Vertices);

        // Note: using upload heaps to transfer static data like vert buffers is not
        // recommended. Every time the GPU needs it, the upload heap will be
        // marshalled over. Please read up on Default Heap usage. An upload heap is
        // used here for code simplicity and because there are very few verts to
        // actually transfer.

        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

        ThrowIfFailed(m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(
            0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(
            0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, g_Vertices, sizeof(g_Vertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(VertexPosColor2);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;

        //----------------------------------------------------------------------------------------------
        // Indices


        const UINT indexBufferSize =
            static_cast<UINT>(sizeof(g_Indicies));

        CD3DX12_HEAP_PROPERTIES heapProperty =
            CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferResource =
            CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        ThrowIfFailed(m_device->CreateCommittedResource(
            &heapProperty, D3D12_HEAP_FLAG_NONE, &bufferResource, //
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_indexBuffer)));

        // Copy the triangle data to the index buffer.
        UINT8* pIndexDataBegin;
        ThrowIfFailed(m_indexBuffer->Map(
            0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
        memcpy(pIndexDataBegin, g_Indicies, indexBufferSize);
        m_indexBuffer->Unmap(0, nullptr);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }
}
