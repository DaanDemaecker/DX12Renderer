// DescriptorAllocation.h

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef _DESCRIPTOR_ALLOCATION_
#define _DESCRIPTOR_ALLOCATION_

// File includes
#include "Includes/DirectXIncludes.h"

// Standard library includes
#include <cstdint>
#include <memory>

namespace DDM
{
	// Class forward declarations
	class DescriptorAllocatorPage;

	class DescriptorAllocation final
	{
	public:
		// Creates a NULL Descriptor
		DescriptorAllocation();

		DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page);


		// Destructor will automatically free allocation
		~DescriptorAllocation();

		// Copies are not allowed.
		DescriptorAllocation(const DescriptorAllocation&) = delete;
		DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

		// Move is allowed.
		DescriptorAllocation(DescriptorAllocation&& allocation);
		DescriptorAllocation& operator=(DescriptorAllocation&& other);

		// Check if this a valid descriptor.
		bool IsNull() const;

		// Get a descriptor at a particular offset in the allocation.
		D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

		// Get the number of (consecutive) handles for this allocation.
		uint32_t GetNumHandles() const;

		// Get the heap that this allocation came from.
		// (For internal use only).
		std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;
	private:
		// Free the descriptor back to the heap it came from.
		void Free();

		// The base descriptor.
		D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor;
		// The number of descriptors in this allocation.
		uint32_t m_NumHandles;
		// The offset to the next descriptor.
		uint32_t m_DescriptorSize;

		// A pointer back to the original page where this allocation came from.
		std::shared_ptr<DescriptorAllocatorPage> m_Page;
	};
}

#endif // !_DESCRIPTOR_ALLOCATION_
