#include "Buffer.h"

DDM::Buffer::Buffer(const std::wstring& name)
	:Resource(name)
{
}

DDM::Buffer::Buffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
	: Resource(resDesc, nullptr, name)
{
}
