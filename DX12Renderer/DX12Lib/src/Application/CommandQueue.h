// CommandQueue.h

/**
* Wrapper class for ID3D12CommandQueue
*/

#ifndef CommandQueueIncluded
#define CommandQueueIncluded

#include <d3D12.h> // For ID3D12CommandQueue, ID3D12Device2, and ID3D12Fence
#include <wrl.h>    // For Microsoft::WRL::ComPtr

#include <cstdint>  // For uint64_t
#include <queue>    // For std::queue
#include <memory>	// for std::shared_ptr

namespace DDM
{
	class CommandList;

	class CommandQueue
	{
	public:
		CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
		virtual ~CommandQueue();

		// Delete default constructor
		CommandQueue() = delete;

		// Delete copy and move methods
		CommandQueue(CommandQueue& other) = delete;
		CommandQueue(CommandQueue&& other) = delete;

		CommandQueue& operator=(CommandQueue& other) = delete;
		CommandQueue& operator=(CommandQueue&& other) = delete;

		 // Returns an available command list from the command queue
		std::shared_ptr<CommandList> GetCommandList();

		// Execute a command list
		// Returns the fence value to wait for for this comand list
		uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList);

		uint64_t Signal();
		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFenceValue(uint64_t fenceValue);
		void Flush();

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;
	
	protected:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
		std::shared_ptr<CommandList> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	private:
		// Keep track of command allocators that are "in-flight"
		struct CommandAllocatorEntry
		{
			uint64_t fenceValue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		};

		using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
		using CommandListQueue = std::queue<std::shared_ptr<CommandList>>;

		D3D12_COMMAND_LIST_TYPE						m_CommandListType;
		Microsoft::WRL::ComPtr<ID3D12Device2>		m_d3d12Device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_d3d12CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12Fence>			m_d3d12Fence;
		HANDLE										m_FenceEvent;
		uint64_t									m_FenceValue;

		CommandAllocatorQueue						m_CommandAllocatorQueue;
		CommandListQueue							m_CommandListQueue;
	};
}


#endif // !CommandQueueIncluded
