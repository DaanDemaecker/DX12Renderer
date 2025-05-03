// CommandQueue.cpp

// Header include
#include "CommandQueue.h"

// File includes
#include "Helpers/Helpers.h"
#include "CommandList.h"
#include "Includes/DXRHelpersIncludes.h"

// Standard library includes
#include <cassert>

DDM::CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
	:m_FenceValue{0},
	m_CommandListType{type},
	m_d3d12Device{device}
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	
	ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	ThrowIfFailed(m_d3d12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

	m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_FenceEvent && "Failed to create fence event handle");
}

DDM::CommandQueue::~CommandQueue()
{

}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DDM::CommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

std::shared_ptr<DDM::CommandList> DDM::CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{
	auto commandList = std::make_shared<DDM::CommandList>(m_CommandListType);

	return commandList;
}

std::shared_ptr<DDM::CommandList> DDM::CommandQueue::GetCommandList()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	std::shared_ptr<CommandList> commandList;

	if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
	{
		commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
		m_CommandAllocatorQueue.pop();

		ThrowIfFailed(commandAllocator->Reset());
	}
	else
	{
		commandAllocator = CreateCommandAllocator();
	}

	if (!m_CommandListQueue.empty())
	{
		commandList = m_CommandListQueue.front();
		m_CommandListQueue.pop();

		ThrowIfFailed(commandList->GetGraphicsCommandList()->Reset(commandAllocator.Get(), nullptr));
	}
	else
	{
		commandList = CreateCommandList(commandAllocator);
	}

	// Associate the command allocator with the command list so that it can be
	// retrieved when the command list is executed
	ThrowIfFailed(commandList->GetGraphicsCommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

	return commandList;

}

uint64_t DDM::CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList)
{
	auto d3dcommandList = commandList->GetGraphicsCommandList();

	d3dcommandList->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT dataSize = sizeof(commandAllocator);
	ThrowIfFailed(d3dcommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	ID3D12CommandList* const ppCommandLists[] = {
		d3dcommandList.Get()
	};

	m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	m_CommandListQueue.push(commandList);

	// The ownership of the command allocator has been transferred
	// in the ocmmand allocator queue. It is safe to release the reference
	// in this temporary COM pointer here
	commandAllocator->Release();

	return fenceValue;
}

uint64_t DDM::CommandQueue::Signal()
{
	uint64_t fenceValueForSignal = ++m_FenceValue;
	ThrowIfFailed(m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

bool DDM::CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
	return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void DDM::CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
	if (m_d3d12Fence->GetCompletedValue() < m_FenceValue)
	{
		ThrowIfFailed(m_d3d12Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent));
		::WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(1'000'000'000));
	}
}

void DDM::CommandQueue::Flush()
{
	uint64_t fenceValueForSignal = Signal();
	WaitForFenceValue(fenceValueForSignal);
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> DDM::CommandQueue::GetD3D12CommandQueue() const
{
	return m_d3d12CommandQueue;
}
