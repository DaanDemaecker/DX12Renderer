// DescriptorAllocatorPage.h

#ifndef _DESCRIPTOR_ALLOCATOR_PAGE_
#define _DESCRIPTOR_ALLOCATOR_PAGE_

namespace DDM
{
	class DescriptorAllocatorPage final
	{
	public:
		DescriptorAllocatorPage();
		~DescriptorAllocatorPage();

		DescriptorAllocatorPage(DescriptorAllocatorPage& other) = delete;
		DescriptorAllocatorPage(DescriptorAllocatorPage&& other) = delete;

		DescriptorAllocatorPage& operator=(DescriptorAllocatorPage& other) = delete;
		DescriptorAllocatorPage& operator=(DescriptorAllocatorPage&& other) = delete;

	private:

	};
}

#endif // !_DESCRIPTOR_ALLOCATOR_PAGE_
