// Windows runtime library. Needed for Microsoft::WRL::CompPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <fstream>

// Helpers include
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"

// DirectX12 include
#include "Includes/DirectXIncludes.h"

// File includes
#include "Application/Application.h"
#include "Tutorial3.h"



void OpenConsole()
{
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout); // Redirect stdout
    freopen_s(&fp, "CONOUT$", "w", stderr); // Redirect stderr
    freopen_s(&fp, "CONIN$", "r", stdin);   // Redirect stdin
    std::cout << "Console attached!\n";
}

int CALLBACK WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
#if defined(_DEBUG)
    OpenConsole();
#endif

    DDM::Application::Get().Initialize(hInstance);

    {
        auto pGame = std::make_shared<DDM::Tutorial3>(L"DirectX tutorial 3", 1080, 720, true);

        DDM::Application::Get().Run(pGame);
    }

    DDM::Application::Get().ShutDown();

    return 0;
}