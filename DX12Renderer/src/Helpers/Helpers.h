#ifndef HelpersIncluded
#define HelpersIncluded

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stdexcept>

#include <cstddef>  // for size_t
#include <cstdint>  // optional


// From DXSampleHelper.h
// Source: https://github.com/Microsoft/DirectX-Graphics-Samples
inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

namespace Math {
	template <typename T>
	constexpr T AlignUp(T value, T alignment) {
		return (value + alignment - 1) & ~(alignment - 1);
	}
}

#endif // !HelpersIncluded
