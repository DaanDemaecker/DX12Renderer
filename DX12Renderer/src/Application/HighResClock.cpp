// HighResClock.cpp

// Header include
#include "HighResClock.h"

DDM::HighResClock::HighResClock()
{
	m_T0 = std::chrono::high_resolution_clock::now();
}

DDM::HighResClock::~HighResClock()
{
}

void DDM::HighResClock::Tick()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    m_DeltaTime = t1 - m_T0;
    m_TotalTime += m_DeltaTime;
    m_T0 = t1;
}

void DDM::HighResClock::Reset()
{
    m_T0 = std::chrono::high_resolution_clock::now();
    m_DeltaTime = std::chrono::high_resolution_clock::duration();
    m_TotalTime = std::chrono::high_resolution_clock::duration();
}
