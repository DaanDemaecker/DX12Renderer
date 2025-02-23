// FenceObject.cpp

// Header include
#include "FenceObject.h"

// File includes
#include "Helpers/Helpers.h"

// Standard library includes
#include <chrono>

DDM::FenceObject::FenceObject(ComPtr<ID3D12Device2> device, uint8_t numFrames) : m_NumFrames{numFrames}
{
    m_Fence = CreateFence(device);

    m_FenceEvent = CreateEventHandle();
}

DDM::FenceObject::~FenceObject()
{
}

uint64_t DDM::FenceObject::Signal(ComPtr<ID3D12CommandQueue> commandQueue)
{
    uint64_t fenceValueForSignal = ++m_FenceValue;
    ThrowIfFailed(commandQueue->Signal(m_Fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void DDM::FenceObject::Flush(ComPtr<ID3D12CommandQueue> commandQueue)
{
    uint64_t fenceValueForSignal = Signal(commandQueue);
    WaitForFenceValue(fenceValueForSignal);
}

void DDM::FenceObject::WaitForFenceValue(uint64_t value, std::chrono::milliseconds duration)
{
    if (m_Fence->GetCompletedValue() < m_FenceValue)
    {
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent));
        ::WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void DDM::FenceObject::CloseHandle(ComPtr<ID3D12CommandQueue> commandQueue)
{
    Flush(commandQueue);

    ::CloseHandle(m_FenceEvent);
}


ComPtr<ID3D12Fence> DDM::FenceObject::CreateFence(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12Fence> fence;

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

HANDLE DDM::FenceObject::CreateEventHandle()
{
    HANDLE fenceEvent;

    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}