// DescriptorAllocator.h

#ifndef _DESCRIPTOR_ALLOCATOR_
#define _DESCRIPTOR_ALLOCATOR_

namespace DDM
{
	class DescriptorAllocator final
	{
	public:
		DescriptorAllocator();
		~DescriptorAllocator();

		DescriptorAllocator(DescriptorAllocator& other) = delete;
		DescriptorAllocator(DescriptorAllocator&& other) = delete;

		DescriptorAllocator& operator=(DescriptorAllocator& other) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&& other) = delete;

	private:

	};
}

#endif // !_DESCRIPTOR_ALLOCATOR_
