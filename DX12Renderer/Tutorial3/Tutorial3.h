// Tutorial3.h

// This game class is used for the DirectX12 tutorial

#ifndef _TUTORIAL_3_
#define _TUTORIAL_3_

// Parent include
#include "Games/Game.h"

// File includes
#include "Application/Window.h"
#include "Includes/DirectXIncludes.h"
#include "Application/DataTypes/Mesh.h"

namespace DDM
{
	class Tutorial3 : public Game
	{
	public:
		// Constructor
		Tutorial3(const std::wstring& name, int width, int height, bool vSync = false);

		// Delete default constructor
		Tutorial3() = delete;

		// Delete copy and move functions
		Tutorial3(Tutorial3& other) = delete;
		Tutorial3(Tutorial3&& other) = delete;

		Tutorial3& operator=(Tutorial3& other) = delete;
		Tutorial3& operator=(Tutorial3&& other) = delete;

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
			D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

		// Create a GPU buffer
		void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
			ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
			size_t numElements, size_t elementSize, const void* bufferData,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

		// Resize the depth buffer to match the size of the client area
		void ResizeDepthBuffer(int width, int height);

		std::vector<uint64_t> m_FenceValues = {};

		// Depth buffer
		ComPtr<ID3D12Resource> m_DepthBuffer;
		// Descriptor heap for depth buffer
		ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

		// Root signature
		ComPtr<ID3D12RootSignature> m_RootSignature;

		// Pipeline state object
		ComPtr<ID3D12PipelineState> m_PipelineState;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		float m_FoV;

		std::unique_ptr<Mesh> m_pMesh1;
		std::unique_ptr<Mesh> m_pMesh2;

		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ProjectionMatrix;

		bool m_ContentLoaded;
	};
}


#endif // !_TUTORIAL_3_
