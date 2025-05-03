// RayTracingScene.h

// This class will contain a small scene that is rendered using raytracing

#ifndef _RAY_TRACING_SCENE_
#define _RAY_TRACING_SCENE_

// Parent include
#include "Games/Game.h"

// File includes
#include "Application/Window.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/DXRHelpersIncludes.h"
#include "Application/CommandList.h"
#include "Application/CommandQueue.h"

// Standard library includes
#include <dxcapi.h>
#include <vector>


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
		bool m_UseRayTracing = false;

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
		ComPtr<ID3D12Device5> m_Device;

		void SetupRasterizer();

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

		CommandQueue* m_CommandQueue;

		std::vector<uint64_t> m_FenceValues = {};

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

		// Pipeline state object
		ComPtr<ID3D12PipelineState> m_PipelineState;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		float m_FoV;

		DirectX::XMMATRIX m_ModelMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ProjectionMatrix;

		bool m_ContentLoaded;


		void SetupRaytracer();
		 
		// #DXR
		struct AccelerationStructureBuffers
		{
			ComPtr<ID3D12Resource> pScratch;      // Scratch memory for AS builder
			ComPtr<ID3D12Resource> pResult;       // Where the AS is
			ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances
		};

		ComPtr<ID3D12Resource> m_bottomLevelAS; // Storage for the bottom Level AS

		nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;
		AccelerationStructureBuffers m_topLevelASBuffers;
		std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> m_instances;

		/// Create the acceleration structure of an instance
		///
		/// \param     vVertexBuffers : pair of buffer and vertex count
		/// \return    AccelerationStructureBuffers for TLAS
		AccelerationStructureBuffers CreateBottomLevelAS( std::shared_ptr<CommandList> commandList,
			std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers,
			std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vIndexBuffers =
			{});

		/// Create the main acceleration structure that holds
		/// all instances of the scene
		/// \param     instances : pair of BLAS and transform
		// #DXR Extra - Refitting
		/// \param     updateOnly: if true, perform a refit instead of a full build
		void CreateTopLevelAS(std::shared_ptr<CommandList> commandList,
			const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>
			& instances,
			bool updateOnly = false);

		/// Create all acceleration structures, bottom and top
		void CreateAccelerationStructures(std::shared_ptr<CommandList> commandList);

		// #DXR
		ComPtr<ID3D12RootSignature> CreateRayGenSignature();
		ComPtr<ID3D12RootSignature> CreateMissSignature();
		ComPtr<ID3D12RootSignature> CreateHitSignature();

		void CreateRaytracingPipeline();

		ComPtr<IDxcBlob> m_rayGenLibrary;
		ComPtr<IDxcBlob> m_hitLibrary;
		ComPtr<IDxcBlob> m_missLibrary;

		ComPtr<ID3D12RootSignature> m_rayGenSignature;
		ComPtr<ID3D12RootSignature> m_hitSignature;
		ComPtr<ID3D12RootSignature> m_missSignature;

		// Ray tracing pipeline state
		ComPtr<ID3D12StateObject> m_rtStateObject;
		// Ray tracing pipeline state properties, retaining the shader identifiers
		// to use in the Shader Binding Table
		ComPtr<ID3D12StateObjectProperties> m_rtStateObjectProps;

		// #DXR
		void CreateRaytracingOutputBuffer();
		void CreateShaderResourceHeap();
		ComPtr<ID3D12Resource> m_outputResource;
		ComPtr<ID3D12DescriptorHeap> m_srvUavHeap;
	};
}
#endif // !_RAY_TRACING_SCENE_
