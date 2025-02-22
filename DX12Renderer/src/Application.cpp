// Applicatoin.cpp

// Header include
#include "Application.h"

// File includes
#include "Window.h" // For DDM::Window class


DDM::Application::Application(WNDPROC pWndProc, HINSTANCE hInst,
	const wchar_t* windowTitle, uint8_t numFrames)
{
	m_pWindow = std::make_unique<DDM::Window>(pWndProc, hInst, windowTitle, numFrames);
}

DDM::Application::~Application()
{
}

DDM::Window* DDM::Application::GetWindow()
{
	return m_pWindow.get();
}
