// Tutorial2.cpp

// Header include
#include "Tutorial2.h"


using namespace DirectX;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
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


DDM::Tutorial2::Tutorial2(const std::wstring& name, int width, int height, bool vSync)
	:Game(name, width, height, vSync),
    m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
    m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))),
    m_FoV(45.0),
    m_ContentLoaded(false)
{
}

bool DDM::Tutorial2::LoadContent()
{
	return true;
}

void DDM::Tutorial2::UnloadContent()
{
}

void DDM::Tutorial2::OnUpdate(UpdateEventArgs& e)
{
}

void DDM::Tutorial2::OnRender(RenderEventArgs& e)
{
}

void DDM::Tutorial2::OnKeyPressed(KeyEventArgs& e)
{
}

void DDM::Tutorial2::OnMouseWheel(MouseWheelEventArgs& e)
{
}

void DDM::Tutorial2::OnResize(ResizeEventArgs& e)
{
}

void DDM::Tutorial2::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
}

void DDM::Tutorial2::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
}

void DDM::Tutorial2::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT demth)
{
}

void DDM::Tutorial2::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
}

void DDM::Tutorial2::ResizeDepthBuffer(int width, int height)
{
}
