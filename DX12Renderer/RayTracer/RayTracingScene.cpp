// RayTracingScene.cpp

// Header include
#include "RayTracingScene.h"

// File includes
#include "Application/Application.h"
#include "Helpers/Helpers.h"
#include "Application/CommandList.h"
#include "Includes/DXRHelpersIncludes.h"

// Standard library includes
#include <iostream> // For std::cout
#include <cstdint>
#include <algorithm> // For std::min and std::max.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

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

static uint32_t g_Indicies[36] =
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
    m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
    m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))),
    m_FoV(45.0),
    m_ContentLoaded(false)
{
    m_FenceValues.resize(Application::Get().FrameCount());

    m_CommandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_Device = Application::Get().GetDevice();
}

bool DDM::RayTracingScene::LoadContent()
{
    SetupRasterizer();

    SetupRaytracer();

    return true;
}

void DDM::RayTracingScene::UnloadContent()
{
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->GetCommandList();
    auto d3dCommandList = commandList->GetGraphicsCommandList();

    // Upload vertex buffer data.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(d3dCommandList,
        &m_VertexBuffer, &intermediateVertexBuffer,
        _countof(g_Vertices), sizeof(VertexPosColor2), g_Vertices);

    // Create the vertex buffer view.
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor2);

    // Upload index buffer data.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(d3dCommandList,
        &m_IndexBuffer, &intermediateIndexBuffer,
        _countof(g_Indicies), sizeof(uint32_t), g_Indicies);

    // Create index buffer view.
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_IndexBufferView.SizeInBytes = sizeof(g_Indicies);

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"Resources/Shaders/Default_VS.cso", &vertexShaderBlob));

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"Resources/Shaders/Default_PS.cso", &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(m_Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(m_Device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_ContentLoaded = true;

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());
}

void DDM::RayTracingScene::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    Game::OnUpdate(e);

    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        std::cout << "FPS: " << fps << std::endl;

        frameCount = 0;
        totalTime = 0.0;
    }

    // Update the model matrix.
    float angle = static_cast<float>(e.TotalTime * 90.0);
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    // Update the view matrix.
    const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix.
    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}

void DDM::RayTracingScene::OnRender(RenderEventArgs& e)
{
    Game::OnRender(e);

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();
    auto d3dCommandList = commandList->GetGraphicsCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto rtv = m_pWindow->GetCurrentRenderTargetView();
    auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(d3dCommandList, backBuffer,
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        ClearRTV(d3dCommandList, rtv, clearColor);
        ClearDepth(d3dCommandList, dsv);
    }

    if (!m_UseRayTracing)
    {
        d3dCommandList->SetPipelineState(m_PipelineState.Get());
        d3dCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

        d3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3dCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
        d3dCommandList->IASetIndexBuffer(&m_IndexBufferView);

        d3dCommandList->RSSetViewports(1, &m_Viewport);
        d3dCommandList->RSSetScissorRects(1, &m_ScissorRect);

        d3dCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

        // Update the MVP matrix
        XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
        mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
        d3dCommandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

        d3dCommandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);
    }
    else
    {

    }

    // Present
    {
        TransitionResource(d3dCommandList, backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

        currentBackBufferIndex = m_pWindow->Present();

        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void DDM::RayTracingScene::OnKeyPressed(KeyEventArgs& e)
{
    Game::OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
        PostQuitMessage(0);
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
            m_pWindow->ToggleFullscreen();
        }
        break;
    case KeyCode::F11:
        m_pWindow->ToggleFullscreen();
        break;
    case KeyCode::V:
        m_pWindow->ToggleVsync();
        break;
    case KeyCode::Space:
        m_UseRayTracing = !m_UseRayTracing;
        break;
    }
}

void DDM::RayTracingScene::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_FoV -= e.WheelDelta;
    m_FoV = std::clamp(m_FoV, 12.0f, 90.0f);

    std::cout << "FOV: " << m_FoV << std::endl;
}

void DDM::RayTracingScene::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        Game::OnResize(e);

        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.Width), static_cast<float>(e.Height));

        ResizeDepthBuffer(e.Width, e.Height);
    }
}

void DDM::RayTracingScene::SetupRasterizer()
{
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    auto commandList = commandQueue->GetCommandList();
    auto d3dCommandList = commandList->GetGraphicsCommandList();

    // Upload vertex buffer data.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(d3dCommandList,
        &m_VertexBuffer, &intermediateVertexBuffer,
        _countof(g_Vertices), sizeof(VertexPosColor2), g_Vertices);

    // Create the vertex buffer view.
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor2);

    // Upload index buffer data.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(d3dCommandList,
        &m_IndexBuffer, &intermediateIndexBuffer,
        _countof(g_Indicies), sizeof(uint32_t), g_Indicies);

    // Create index buffer view.
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_IndexBufferView.SizeInBytes = sizeof(g_Indicies);

    // Create the descriptor heap for the depth-stencil view.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"Resources/Shaders/Default_VS.cso", &vertexShaderBlob));

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"Resources/Shaders/Default_PS.cso", &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(m_Device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(m_Device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_ContentLoaded = true;

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());
}

void DDM::RayTracingScene::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void DDM::RayTracingScene::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void DDM::RayTracingScene::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void DDM::RayTracingScene::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = DDM::Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap
    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));

    if (bufferData)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc2 = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc2,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void DDM::RayTracingScene::ResizeDepthBuffer(int width, int height)
{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        Application::Get().Flush();

        auto newWidth = std::max(1, width);
        auto newHeight = std::max(1, height);

        auto device = Application::Get().GetDevice();

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&m_DepthBuffer)
        ));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv,
            m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void DDM::RayTracingScene::SetupRaytracer()
{
    auto commandList = m_CommandQueue->GetCommandList();

    CreateAccelerationStructures(commandList);

    CreateRaytracingPipeline();

    CreateRaytracingOutputBuffer();

    CreateShaderResourceHeap();

	CreateShaderBindingTable();
}


//-----------------------------------------------------------------------------
//
// Create a bottom-level acceleration structure based on a list of vertex
// buffers in GPU memory along with their vertex count. The build is then done
// in 3 steps: gathering the geometry, computing the sizes of the required
// buffers, and building the actual AS
//
DDM::RayTracingScene::AccelerationStructureBuffers DDM::RayTracingScene::CreateBottomLevelAS(
    std::shared_ptr<CommandList> commandList,
    std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
    std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    for (size_t i = 0; i < vVertexBuffers.size(); i++) {
        if (i < vIndexBuffers.size() && vIndexBuffers[i].second > 0) {
            bottomLevelAS.AddVertexBuffer(
                vVertexBuffers[i].first.Get(), 0,
                vVertexBuffers[i].second, sizeof(VertexPosColor2),
                vIndexBuffers[i].first.Get(), 0,
                vIndexBuffers[i].second, nullptr, 0, true);
        }
        else {
            bottomLevelAS.AddVertexBuffer(
                vVertexBuffers[i].first.Get(), 0,
                vVertexBuffers[i].second, sizeof(VertexPosColor2), 0, 0);
        }
    }

    UINT64 scratchSizeInBytes = 0;
    UINT64 resultSizeInBytes = 0;

    bottomLevelAS.ComputeASBufferSizes(m_Device.Get(), false, &scratchSizeInBytes, &resultSizeInBytes);

    AccelerationStructureBuffers buffers;

    // Create scratch buffer in COMMON state
    buffers.pScratch = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), scratchSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);

    // Create result buffer in COMMON state (no need to transition it manually)
    buffers.pResult = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), resultSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);

    // Transition scratch buffer only
    auto cmdList = commandList->GetGraphicsCommandList();
    CD3DX12_RESOURCE_BARRIER scratchBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        buffers.pScratch.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    cmdList->ResourceBarrier(1, &scratchBarrier);

    // Build BLAS - result buffer will internally go into the correct state
    bottomLevelAS.Generate(
        cmdList.Get(),
        buffers.pScratch.Get(),
        buffers.pResult.Get(),
        false, nullptr);

    return buffers;
}



//-----------------------------------------------------------------------------
// Create the main acceleration structure that holds all instances of the scene.
// Similarly to the bottom-level AS generation, it is done in 3 steps: gathering
// the instances, computing the memory requirements for the AS, and building the
// AS itself
//
void DDM::RayTracingScene::CreateTopLevelAS(std::shared_ptr<CommandList> commandList, const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances, bool updateOnly)
{
    auto d3dcommandList = commandList->GetGraphicsCommandList();

    // Gather all the instances into the builder helper
        for (size_t i = 0; i < instances.size(); i++) {
            m_topLevelASGenerator.AddInstance(instances[i].first.Get(),
                instances[i].second, static_cast<UINT>(i),
                static_cast<UINT>(0));
        }

    // As for the bottom-level AS, the building the AS requires some scratch space
    // to store temporary data in addition to the actual AS. In the case of the
    // top-level AS, the instance descriptors also need to be stored in GPU
    // memory. This call outputs the memory requirements for each (scratch,
    // results, instance descriptors) so that the application can allocate the
    // corresponding memory
    UINT64 scratchSize, resultSize, instanceDescsSize;

    m_topLevelASGenerator.ComputeASBufferSizes(m_Device.Get(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    // Create the scratch and result buffers. Since the build is all done on GPU,
    // those can be allocated on the default heap
    m_topLevelASBuffers.pScratch = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COMMON,
        nv_helpers_dx12::kDefaultHeapProps);

    
    auto commandList2 = m_CommandQueue->GetCommandList();
    TransitionResource(commandList2->GetGraphicsCommandList(), m_topLevelASBuffers.pScratch, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    auto fenceValue = m_CommandQueue->ExecuteCommandList(commandList2);
    m_CommandQueue->WaitForFenceValue(fenceValue);


    m_topLevelASBuffers.pResult = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nv_helpers_dx12::kDefaultHeapProps);


    // The buffer describing the instances: ID, shader binding information,
    // matrices ... Those will be copied into the buffer by the helper through
    // mapping, so the buffer has to be allocated on the upload heap.
    m_topLevelASBuffers.pInstanceDesc = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

    // After all the buffers are allocated, or if only an update is required, we
    // can build the acceleration structure. Note that in the case of the update
    // we also pass the existing AS as the 'previous' AS, so that it can be
    // refitted in place.
    m_topLevelASGenerator.Generate(d3dcommandList.Get(),
        m_topLevelASBuffers.pScratch.Get(),
        m_topLevelASBuffers.pResult.Get(),
        m_topLevelASBuffers.pInstanceDesc.Get());
}

//-----------------------------------------------------------------------------
//
// Combine the BLAS and TLAS builds to construct the entire acceleration
// structure required to raytrace the scene
//
void DDM::RayTracingScene::CreateAccelerationStructures(std::shared_ptr<CommandList> commandList)
{
    // Build the bottom AS from the Triangle vertex buffer
    AccelerationStructureBuffers bottomLevelBuffers =
        CreateBottomLevelAS( commandList,{ {m_VertexBuffer.Get(), _countof(g_Vertices)}}, {{m_IndexBuffer.Get(), _countof(g_Indicies) } });

    // Just one instance for now
    m_instances = { {bottomLevelBuffers.pResult, XMMatrixIdentity()} };
    CreateTopLevelAS(commandList, m_instances);

    // Flush the command list and wait for it to finish
    //commandList->GetGraphicsCommandList()->Close();
    m_CommandQueue->ExecuteCommandList(commandList);
    m_FenceValues[0] = m_CommandQueue->Signal();
    m_CommandQueue->WaitForFenceValue(m_FenceValues[0]);

    // Store the AS buffers. The rest of the buffers will be released once we exit
    // the function
    m_bottomLevelAS = bottomLevelBuffers.pResult;
}

ComPtr<ID3D12RootSignature> DDM::RayTracingScene::CreateRayGenSignature()
{
    nv_helpers_dx12::RootSignatureGenerator rsc;
    rsc.AddHeapRangesParameter(
        { {0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,
          D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
          0 /*heap slot where the UAV is defined*/},
         {0 /*t0*/, 1, 0,
          D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,
          1} });

    return rsc.Generate(m_Device.Get(), true);
}

ComPtr<ID3D12RootSignature> DDM::RayTracingScene::CreateMissSignature()
{
    nv_helpers_dx12::RootSignatureGenerator rsc;
    return rsc.Generate(m_Device.Get(), true);
}

ComPtr<ID3D12RootSignature> DDM::RayTracingScene::CreateHitSignature()
{
    nv_helpers_dx12::RootSignatureGenerator rsc;
    return rsc.Generate(m_Device.Get(), true);
}

void DDM::RayTracingScene::CreateRaytracingPipeline()
{
    nv_helpers_dx12::RayTracingPipelineGenerator pipeline(m_Device.Get());

    // The pipeline contains the DXIL code of all the shaders potentially executed
    // during the raytracing process. This section compiles the HLSL code into a
    // set of DXIL libraries. We chose to separate the code in several libraries
    // by semantic (ray generation, hit, miss) for clarity. Any code layout can be
    // used.
    ComPtr<IDxcBlobEncoding> blobEncoding;
    ComPtr<IDxcUtils> dxcUtils;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));

    UINT32 codePage = CP_UTF8;
    ThrowIfFailed(dxcUtils->LoadFile(L"Resources/Shaders/Raygen_RGS.cso", &codePage, &blobEncoding));

    // Cast to IDxcBlob
    ThrowIfFailed(blobEncoding.As(&m_rayGenLibrary));

    ThrowIfFailed(dxcUtils->LoadFile(L"Resources/Shaders/Miss_MSS.cso", &codePage, &blobEncoding));

    // Cast to IDxcBlob
    ThrowIfFailed(blobEncoding.As(&m_missLibrary));

    ThrowIfFailed(dxcUtils->LoadFile(L"Resources/Shaders/Hit_CHS.cso", &codePage, &blobEncoding));

    // Cast to IDxcBlob
    ThrowIfFailed(blobEncoding.As(&m_hitLibrary));

    // In a way similar to DLLs, each library is associated with a number of
    // exported symbols. This
     // has to be done explicitly in the lines below. Note that a single library
    // can contain an arbitrary number of symbols, whose semantic is given in HLSL
    // using the [shader("xxx")] syntax
    pipeline.AddLibrary(m_rayGenLibrary.Get(), { L"RayGen" });
    pipeline.AddLibrary(m_missLibrary.Get(), { L"Miss" });
    pipeline.AddLibrary(m_hitLibrary.Get(), { L"ClosestHit" });

    // To be used, each DX12 shader needs a root signature defining which
    // parameters and buffers will be accessed.
    m_rayGenSignature = CreateRayGenSignature();
    m_missSignature = CreateMissSignature();
    m_hitSignature = CreateHitSignature();  

    // 3 different shaders can be invoked to obtain an intersection: an
    // intersection shader is called
    // when hitting the bounding box of non-triangular geometry. This is beyond
    // the scope of this tutorial. An any-hit shader is called on potential
    // intersections. This shader can, for example, perform alpha-testing and
    // discard some intersections. Finally, the closest-hit program is invoked on
    // the intersection point closest to the ray origin. Those 3 shaders are bound
    // together into a hit group.

    // Note that for triangular geometry the intersection shader is built-in. An
    // empty any-hit shader is also defined by default, so in our simple case each
    // hit group contains only the closest hit shader. Note that since the
    // exported symbols are defined above the shaders can be simply referred to by
    // name.

    // Hit group for the triangles, with a shader simply interpolating vertex
    // colors
    pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");

    // The following section associates the root signature to each shader. Note
    // that we can explicitly show that some shaders share the same root signature
    // (eg. Miss and ShadowMiss). Note that the hit shaders are now only referred
    // to as hit groups, meaning that the underlying intersection, any-hit and
    // closest-hit shaders share the same root signature.
    pipeline.AddRootSignatureAssociation(m_rayGenSignature.Get(), { L"RayGen" });
    pipeline.AddRootSignatureAssociation(m_missSignature.Get(), { L"Miss" });
    pipeline.AddRootSignatureAssociation(m_hitSignature.Get(), { L"HitGroup" });

    // The payload size defines the maximum size of the data carried by the rays,
  // ie. the the data
  // exchanged between shaders, such as the HitInfo structure in the HLSL code.
  // It is important to keep this value as low as possible as a too high value
  // would result in unnecessary memory consumption and cache trashing.
    pipeline.SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance

    // Upon hitting a surface, DXR can provide several attributes to the hit. In
    // our sample we just use the barycentric coordinates defined by the weights
    // u,v of the last two vertices of the triangle. The actual barycentrics can
    // be obtained using float3 barycentrics = float3(1.f-u-v, u, v);
    pipeline.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates

    // The raytracing process can shoot rays from existing hit points, resulting
    // in nested TraceRay calls. Our sample code traces only primary rays, which
    // then requires a trace depth of 1. Note that this recursion depth should be
    // kept to a minimum for best performance. Path tracing algorithms can be
    // easily flattened into a simple loop in the ray generation.
    pipeline.SetMaxRecursionDepth(1);

    // Compile the pipeline for execution on the GPU
    m_rtStateObject = pipeline.Generate();

    // Cast the state object into a properties object, allowing to later access
    // the shader pointers by name
    ThrowIfFailed(
        m_rtStateObject->QueryInterface(IID_PPV_ARGS(&m_rtStateObjectProps)));
}

void DDM::RayTracingScene::CreateRaytracingOutputBuffer()
{
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.DepthOrArraySize = 1;
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    // The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB
    // formats cannot be used with UAVs. For accuracy we should convert to sRGB
    // ourselves in the shader
    resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    resDesc.Width = GetClientWidth();
    resDesc.Height = GetClientHeight();
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    ThrowIfFailed(m_Device->CreateCommittedResource(
        &nv_helpers_dx12::kDefaultHeapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr,
        IID_PPV_ARGS(&m_outputResource)));
}

void DDM::RayTracingScene::CreateShaderResourceHeap()
{
    // Create a SRV/UAV/CBV descriptor heap. We need 2 entries - 1 UAV for the
  // raytracing output and 1 SRV for the TLAS
    m_srvUavHeap = nv_helpers_dx12::CreateDescriptorHeap(
        m_Device.Get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

    // Get a handle to the heap memory on the CPU side, to be able to write the
    // descriptors directly
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle =
        m_srvUavHeap->GetCPUDescriptorHandleForHeapStart();

    // Create the UAV. Based on the root signature we created it is the first
    // entry. The Create*View methods write the view information directly into
    // srvHandle
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    m_Device->CreateUnorderedAccessView(m_outputResource.Get(), nullptr, &uavDesc,
        srvHandle);

    // Add the Top Level AS SRV right after the raytracing output buffer
    srvHandle.ptr += m_Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location =
        m_topLevelASBuffers.pResult->GetGPUVirtualAddress();
    // Write the acceleration structure view in the heap
    m_Device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
}

void DDM::RayTracingScene::CreateShaderBindingTable()
{
    // The SBT helper class collects calls to Add*Program.  If called several
  // times, the helper must be emptied before re-adding shaders.
    m_sbtHelper.Reset();

    // The pointer to the beginning of the heap is the only parameter required by
    // shaders without root parameters
    D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle =
        m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();

    // The helper treats both root parameter pointers and heap pointers as void*,
  // while DX12 uses the
  // D3D12_GPU_DESCRIPTOR_HANDLE to define heap pointers. The pointer in this
  // struct is a UINT64, which then has to be reinterpreted as a pointer.
    auto heapPointer = static_cast<UINT64>(srvUavHeapHandle.ptr);

    auto heapPointers = std::vector<void*>{ reinterpret_cast<void*>(heapPointer) };
    // The ray generation only uses heap data
    m_sbtHelper.AddRayGenerationProgram(L"RayGen", heapPointers);

    // The miss and hit shaders do not access any external resources: instead they
    // communicate their results through the ray payload
    m_sbtHelper.AddMissProgram(L"Miss", {});

    // Adding the triangle hit shader
    m_sbtHelper.AddHitGroup(L"HitGroup", {});

    // Compute the size of the SBT given the number of shaders and their
  // parameters
    uint32_t sbtSize = m_sbtHelper.ComputeSBTSize();

    // Create the SBT on the upload heap. This is required as the helper will use
    // mapping to write the SBT contents. After the SBT compilation it could be
    // copied to the default heap for performance.
    m_sbtStorage = nv_helpers_dx12::CreateBuffer(
        m_Device.Get(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);
    if (!m_sbtStorage) {
        throw std::logic_error("Could not allocate the shader binding table");
    }

    // Compile the SBT from the shader and parameters info
    m_sbtHelper.Generate(m_sbtStorage.Get(), m_rtStateObjectProps.Get());
}
