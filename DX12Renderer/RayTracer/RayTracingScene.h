// RayTracingScene.h

// This class will contain a small scene that is rendered using raytracing

#ifndef _RAY_TRACING_SCENE_
#define _RAY_TRACING_SCENE_

// Parent class include
#include "Games/Game.h"
#include "Includes/DirectXIncludes.h"

// Standard library include
#include <wrl.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace DDM
{
	class RayTracingScene : public Game
	{
	public:
		// Constructor
		RayTracingScene(const std::wstring& name, int width, int height, bool vSync = false);

		// Delete default constructor
		RayTracingScene() = delete;

		// Delete copy and move functions
		RayTracingScene(RayTracingScene& other) = delete;
		RayTracingScene(RayTracingScene&& other) = delete;

		RayTracingScene& operator=(RayTracingScene& other) = delete;
		RayTracingScene& operator=(RayTracingScene&& other) = delete;

		// Load content required
		virtual bool LoadContent() override;

		// Unloade content loaded in LoadContent method
		virtual void UnloadContent() override;

	protected:
		// Render stuff
		virtual void OnRender(RenderEventArgs& e) override;

		void LoadAssets();

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12Device5> m_device;
		std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList4> m_commandList;
		UINT m_rtvDescriptorSize;

		// App resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

		// Synchronization objects.
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;
	};
}

#endif // !_RAY_TRACING_SCENE_
