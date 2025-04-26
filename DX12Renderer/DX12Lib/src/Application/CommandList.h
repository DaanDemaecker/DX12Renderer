// CommandList.h

#ifndef _COMMAND_LIST_
#define _COMMAND_LIST_

// File includes
#include "Includes/DirectXIncludes.h"

// Standard library includes
#include <wrl.h>

namespace DDM
{
	// Class forward declarations
	class DynamicDescriptorHeap;

	class CommandList final
	{
	public:
		CommandList(D3D12_COMMAND_LIST_TYPE type);
		virtual ~CommandList();
		
		~CommandList();

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
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const
		{
			return m_d3d12CommandList;
		}

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);

	private:
		// Binds the current descriptor heaps to the command list.
		void BindDescriptorHeaps();

		D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_d3d12CommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

		// Keep track of the currently bound root signatures to minimize root
		// signature changes.
		ID3D12RootSignature* m_RootSignature;

		// Keep track of the currently bound descriptor heaps. Only change descriptor 
		// heaps if they are different than the currently bound descriptor heaps.
		ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// The dynamic descriptor heap allows for descriptors to be staged before
		// being committed to the command list. Dynamic descriptors need to be
		// committed before a Draw or Dispatch.
		std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	};
}


#endif // _COMMAND_LIST_