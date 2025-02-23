// FenceObject.h

#ifndef FenceObjectIncluded
#define FenceObjectIncluded

// File includes
#include "Includes/DirectXIncludes.h"

// Standard library includes
#include <wrl.h>
#include <chrono>

using namespace Microsoft::WRL;

namespace DDM
{
	class FenceObject
	{
	public:
		FenceObject(ComPtr<ID3D12Device2> device, uint8_t numFrames);

		~FenceObject();

		FenceObject(FenceObject& other) = delete;
		FenceObject(FenceObject&& other) = delete;

		FenceObject& operator=(FenceObject& other) = delete;
		FenceObject& operator=(FenceObject&& other) = delete;

		void Flush(ComPtr<ID3D12CommandQueue> commandQueue);

		void WaitForFenceValue(uint64_t value, std::chrono::milliseconds duration = static_cast<std::chrono::milliseconds>(100'000'000));

		void CloseHandle(ComPtr<ID3D12CommandQueue> commandQueue);

		uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueuee);
	private:
		// Number of swap chain back buffers
		const uint8_t m_NumFrames;

		// Synchronization objects
		ComPtr<ID3D12Fence> m_Fence;
		uint64_t m_FenceValue = 0;
		HANDLE m_FenceEvent;

		ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);

		HANDLE CreateEventHandle();

	};
}


#endif // !FenceObjectIncluded
