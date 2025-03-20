// Tutorial2.h

// This game class is used for the DirectX12 tutorial

#ifndef _TUTORIAL_2_
#define _TUTORIAL_2_

// Parent include
#include "Game.h"

// File includes
#include "Window.h"
#include "Includes/DirectXIncludes.h"

namespace DDM
{
	class Tutorial2 : public Game
	{
	public:
		// Constructor
		Tutorial2(const std::wstring& name, int width, int height, bool vSync = false);

		// Delete default constructor
		Tutorial2() = delete;

		// Delete copy and move functions
		Tutorial2(Tutorial2& other) = delete;
		Tutorial2(Tutorial2&& other) = delete;
		
		Tutorial2& operator=(Tutorial2& other) = delete;
		Tutorial2& operator=(Tutorial2&& other) = delete;
		
		// Load content required
		virtual bool LoadContent() override;

		// Unloade content loaded in LoadContent method
		virtual void UnloadContent() override;

	protected:
		// Update game logic
		virtual void OnUpdate(UpdateEventArgs& e) override;

		// Render stuff
		virtual void OnRender(RenderEventArgs& e) override;

		// Invoked by registered window when a key is pressed
		// while the window has focus
		virtual void OnKeyPressed(KeyEventArgs& e) override;

		// Invoked when mouse wheel is scrolled while the registered window has focus.
		virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

		// Invoked when window is resized
		virtual void OnResize(ResizeEventArgs& e) override;
	private:

		// Helper functions
		// Transition a resource
		void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
			ComPtr<ID3D12Resource> resource,
			D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

		// Clear a render target view
		void ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
			D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

		// Clear the depth of a depth-stencil view

		void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
			D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT demth = 1.0f);

		// Create a GPU buffer
		void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
			ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
			size_t numElements, size_t elementSize, const void* bufferData,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		// Resize the depth buffer to match the size of the client area
		void ResizeDepthBuffer(int width, int height);

		uint64_t m_FenceValues[Window::BufferCount] = {};

		// Vertex buffer for cube
		ComPtr<ID3D12Resource> m_VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		// Index buffer for the cube
		ComPtr<ID3D12Resource> m_IndexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

		// Depth buffer
		ComPtr<ID3D12Resource> m_DepthBuffer;
		// Descriptor heap for depth buffer
		ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

		// Root signature
		ComPtr<ID3D12RootSignature> m_RootSignature;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		float m_FoV;

		DirectX::XMMATRIX m_ModelMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ProjectionMatrix;

		bool m_ContentLoaded;
	};
}


#endif // !_TUTORIAL_2_
