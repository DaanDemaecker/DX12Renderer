// Game.cpp

// Header include
#include "Game.h"

// File includes
#include "Includes/DirectXIncludes.h"
#include "Application.h"
#include "Window.h"

DDM::Game::Game(const std::wstring& name, int width, int height, bool vSync)
	:m_Name{name}, m_Width{width}, m_Height{height}, m_vSync{vSync}
{

}

DDM::Game::~Game()
{
}

bool DDM::Game::Initialize()
{
	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
	
	m_pWindow->ShowWindow();


	return true;
}

void DDM::Game::Destroy()
{
}

void DDM::Game::OnUpdate(UpdateEventArgs& e)
{
}

void DDM::Game::OnRender(RenderEventArgs& e)
{
}

void DDM::Game::OnKeyPressed(KeyEventArgs& e)
{
}

void DDM::Game::OnKeyReleased(KeyEventArgs& e)
{
}

void DDM::Game::OnMouseMoved(MouseMotionEventArgs& e)
{
}

void DDM::Game::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
}

void DDM::Game::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
}

void DDM::Game::OnMouseWheel(MouseWheelEventArgs& e)
{
}

void DDM::Game::OnResize(ResizeEventArgs& e)
{
}

void DDM::Game::OnWindowDestroy()
{
}
