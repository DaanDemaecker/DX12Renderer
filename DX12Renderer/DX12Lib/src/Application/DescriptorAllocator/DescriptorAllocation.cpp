// DescriptorAllocation.cpp

// Header include
#include "DescriptorAllocation.h"

// File includes
#include "DescriptorAllocatorPage.h"

DDM::DescriptorAllocation::DescriptorAllocation()
{
}

DDM::DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page)
{
}

DDM::DescriptorAllocation::~DescriptorAllocation()
{
}

DDM::DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
{
}

//DDM::DescriptorAllocation& DDM::DescriptorAllocation::operator=(DescriptorAllocation&& other)
//{
//	return nullptr;
//}

bool DDM::DescriptorAllocation::IsNull() const
{
	return false;
}

D3D12_CPU_DESCRIPTOR_HANDLE DDM::DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
{
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

uint32_t DDM::DescriptorAllocation::GetNumHandles() const
{
	return 0;
}

std::shared_ptr<DDM::DescriptorAllocatorPage> DDM::DescriptorAllocation::GetDescriptorAllocatorPage() const
{
	return std::shared_ptr<DescriptorAllocatorPage>();
}

void DDM::DescriptorAllocation::Free()
{
}
