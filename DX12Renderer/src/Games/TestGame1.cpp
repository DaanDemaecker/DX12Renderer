#include "TestGame1.h"
#include "Application/Application.h"
#include "Application/Window.h"

DDM::TestGame1::TestGame1(const std::wstring& name, int width, int height, bool vSync)
	:Game(name, width, height, vSync)
{
	m_pWindow = Application::Get().CreateRenderWindow(name, width, height, vSync);
	m_pWindow->RegisterGame(shared_from_this());
}

DDM::TestGame1::~TestGame1()
{
	m_pWindow->UnRegisterGame(shared_from_this());
}

bool DDM::TestGame1::LoadContent()
{
	return true;
}

void DDM::TestGame1::UnloadContent()
{
}
