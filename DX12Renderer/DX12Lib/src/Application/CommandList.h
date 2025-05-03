// CommandList.h

#ifndef _COMMAND_LIST_
#define _COMMAND_LIST_

// File includes
#include "Includes/DirectXIncludes.h"
#include "Application/UploadBuffer.h"

// Standard library includes
#include <wrl.h>
#include <vector>

namespace DDM
{
	// Class forward declarations
	class DynamicDescriptorHeap;
	class VertexBuffer;
	class IndexBuffer;
	class Buffer;
	class Resource;
	class ResourceStateTracker;
	//class UploadBuffer;

	class CommandList final
	{
	public:
		CommandList(D3D12_COMMAND_LIST_TYPE type);
		virtual ~CommandList();

		// Delete copy and move operations
		CommandList(CommandList& other) = delete;
		CommandList(CommandList&& other) = delete;

		CommandList& operator=(CommandList& other) = delete;
		CommandList& operator=(CommandList&& other) = delete;

		/**
		 * Get the type of command list.
		 */
		D3D12_COMMAND_LIST_TYPE GetCommandListType() const
		{
			return m_d3d12CommandListType;
		}

		/**
		 * Get direct access to the ID3D12GraphicsCommandList2 interface.
		 */
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> GetGraphicsCommandList() const
		{
			return m_d3d12CommandList;
		}

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);

		/**
	 * Copy the contents to a vertex buffer in GPU memory.
	 */
		void CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
		
		template<typename T>
		void CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData)
		{
			CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
		}

		/**
		 * Copy the contents to a index buffer in GPU memory.
		 */
		void CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
		template<typename T>
		void CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData)
		{
			assert(sizeof(T) == 2 || sizeof(T) == 4);

			DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
		}

		/**
		 * Release tracked objects. Useful if the swap chain needs to be resized.
		 */
		void ReleaseTrackedObjects();

		/**
		 * Flush any barriers that have been pushed to the command list.
		 */
		void FlushResourceBarriers();

		void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		void SetVertexBuffer(UINT startSlot, VertexBuffer& vertexBuffer);

		void SetIndexBuffer(IndexBuffer& indexBuffer);

		/**
	 * Draw geometry.
	 */
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);



		/**
		 * Transition a resource to a particular state.
		 *
		 * @param resource The resource to transition.
		 * @param stateAfter The state to transition the resource to. The before state is resolved by the resource state tracker.
		 * @param subresource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES which indicates that all subresources are transitioned to the same state.
		 * @param flushBarriers Force flush any barriers. Resource barriers need to be flushed before a command (draw, dispatch, or copy) that expects the resource to be in a particular state can run.
		 */
		void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
		void TransitionBarrier(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);


	private:
		void TrackResource(Microsoft::WRL::ComPtr<ID3D12Object> object);
		void TrackResource(const Resource& res);

		// Binds the current descriptor heaps to the command list.
		void BindDescriptorHeaps();

		// Copy the contents of a CPU buffer to a GPU buffer (possibly replacing the previous buffer contents).
		void CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);


		D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_d3d12CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

		// Keep track of the currently bound root signatures to minimize root
		// signature changes.
		ID3D12RootSignature* m_RootSignature;

		// Resource created in an upload heap. Useful for drawing of dynamic geometry
		// or for uploading constant buffer data that changes every draw call.
		std::unique_ptr<UploadBuffer> m_UploadBuffer;

		// Keep track of the currently bound descriptor heaps. Only change descriptor 
		// heaps if they are different than the currently bound descriptor heaps.
		ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// The dynamic descriptor heap allows for descriptors to be staged before
		// being committed to the command list. Dynamic descriptors need to be
		// committed before a Draw or Dispatch.
		std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];


		using TrackedObjects = std::vector < Microsoft::WRL::ComPtr<ID3D12Object> >;
		// Objects that are being tracked by a command list that is "in-flight" on 
		// the command-queue and cannot be deleted. To ensure objects are not deleted 
		// until the command list is finished executing, a reference to the object
		// is stored. The referenced objects are released when the command list is 
		// reset.
		TrackedObjects m_TrackedObjects;

		// Resource state tracker is used by the command list to track (per command list)
		// the current state of a resource. The resource state tracker also tracks the 
		// global state of a resource in order to minimize resource state transitions.
		std::unique_ptr<ResourceStateTracker> m_ResourceStateTracker;
	};
}


#endif // _COMMAND_LIST_