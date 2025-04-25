// DescriptorAllocatorPage.cpp

// Header include
#include "DescriptorAllocatorPage.h"

DDM::DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
}

DDM::DescriptorAllocatorPage::~DescriptorAllocatorPage()
{
}

D3D12_DESCRIPTOR_HEAP_TYPE DDM::DescriptorAllocatorPage::GetHeapType() const
{
	return D3D12_DESCRIPTOR_HEAP_TYPE();
}

bool DDM::DescriptorAllocatorPage::HasSpace(uint32_t numDescriptors) const
{
	return false;
}

uint32_t DDM::DescriptorAllocatorPage::NumFreeHandles() const
{
	return 0;
}

DDM::DescriptorAllocation DDM::DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
{
	return DescriptorAllocation();
}

void DDM::DescriptorAllocatorPage::Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber)
{
}

void DDM::DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{
}

uint32_t DDM::DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	return 0;
}

void DDM::DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{
}

void DDM::DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
{
}

