// DescriptorAllocation.h

#ifndef _DESCRIPTOR_ALLOCATION_
#define _DESCRIPTOR_ALLOCATION_

namespace DDM
{
	class DescriptorAllocation final
	{
	public:
		DescriptorAllocation();
		~DescriptorAllocation();

		DescriptorAllocation(DescriptorAllocation& other) = delete;
		DescriptorAllocation(DescriptorAllocation&& other) = delete;

		DescriptorAllocation& operator=(DescriptorAllocation& other) = delete;
		DescriptorAllocation& operator=(DescriptorAllocation&& other) = delete;

	private:

	};
}

#endif // !_DESCRIPTOR_ALLOCATION_
