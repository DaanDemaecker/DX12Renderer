// HighResClock.h
// This class will serve as a clock to track total time application has been running, elapsed time etc.

#ifndef _HIGH_RES_CLOCK_
#define _HIGH_RES_CLOCK_

// Standard library includes
#include <chrono>

namespace DDM
{
	class HighResClock final
	{
	public:
		// Default constructor
		HighResClock();

		// Destructor
		~HighResClock();

		// Delete copy and move operations
		HighResClock(HighResClock& other) = delete;
		HighResClock(HighResClock&& other) = delete;

		HighResClock& operator=(HighResClock& other) = delete;
		HighResClock& operator=(HighResClock&& other) = delete;


		void Tick();

		void Reset();

		double GetElapsedSec() { return m_DeltaTime.count() * 1e-9; }

		double GetTotalTime() { return m_TotalTime.count() * 1e-9; }

	private:
		std::chrono::high_resolution_clock::time_point m_T0{};

		std::chrono::high_resolution_clock::duration m_DeltaTime{};

		std::chrono::high_resolution_clock::duration m_TotalTime{};
	};
}

#endif // !_HIGH_RES_CLOCK_
