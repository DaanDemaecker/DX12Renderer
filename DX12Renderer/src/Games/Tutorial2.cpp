#include "Tutorial2.h"
// Tutorial2.cpp

DDM::Tutorial2::Tutorial2(const std::wstring& name, int width, int height, bool vSync)
	:Game(name, width, height, vSync)
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
