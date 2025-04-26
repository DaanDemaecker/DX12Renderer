// DescriptorAllocation.cpp

// Header include
#include "DescriptorAllocation.h"

// File includes
#include "DescriptorAllocatorPage.h"
#include "Application/Application.h"

DDM::DescriptorAllocation::DescriptorAllocation()
	: m_Descriptor{ 0 }
	, m_NumHandles(0)
	, m_DescriptorSize(0)
	, m_Page(nullptr)
{
}

DDM::DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page)
	: m_Descriptor(descriptor)
	, m_NumHandles(numHandles)
	, m_DescriptorSize(descriptorSize)
	, m_Page(page)
{
}

DDM::DescriptorAllocation::~DescriptorAllocation()
{
	Free();
}

DDM::DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
	: m_Descriptor(allocation.m_Descriptor)
	, m_NumHandles(allocation.m_NumHandles)
	, m_DescriptorSize(allocation.m_DescriptorSize)
	, m_Page(std::move(allocation.m_Page))
{
	allocation.m_Descriptor.ptr = 0;
	allocation.m_NumHandles = 0;
	allocation.m_DescriptorSize = 0;
}

DDM::DescriptorAllocation& DDM::DescriptorAllocation::operator=(DescriptorAllocation&& other)
{
	// Free this descriptor if it points to anything.
	Free();

	m_Descriptor = other.m_Descriptor;
	m_NumHandles = other.m_NumHandles;
	m_DescriptorSize = other.m_DescriptorSize;
	m_Page = std::move(other.m_Page);

	other.m_Descriptor.ptr = 0;
	other.m_NumHandles = 0;
	other.m_DescriptorSize = 0;

	return *this;
}

bool DDM::DescriptorAllocation::IsNull() const
{
	return m_Descriptor.ptr == 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DDM::DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
{
	assert(offset < m_NumHandles);
	return { m_Descriptor.ptr + (m_DescriptorSize * offset) };
}

uint32_t DDM::DescriptorAllocation::GetNumHandles() const
{
	return m_NumHandles;
}

std::shared_ptr<DDM::DescriptorAllocatorPage> DDM::DescriptorAllocation::GetDescriptorAllocatorPage() const
{
	return m_Page;
}

void DDM::DescriptorAllocation::Free()
{
	if (!IsNull() && m_Page)
	{
		m_Page->Free(std::move(*this), Application::Get().FrameCount());

		m_Descriptor.ptr = 0;
		m_NumHandles = 0;
		m_DescriptorSize = 0;
		m_Page.reset();
	}
}
