#ifndef HelpersIncluded
#define HelpersIncluded

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <stdexcept>

#include <cstddef>  // for size_t
#include <cstdint>  // optional

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace Math {
	template <typename T>
	constexpr T AlignUp(T value, T alignment) {
		return (value + alignment - 1) & ~(alignment - 1);
	}
}

#endif // !HelpersIncluded
