// Windows runtime library. Needed for Microsoft::WRL::CompPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

// Helpers include
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"

// DirectX12 include
#include "Includes/DirectXIncludes.h"

// File includes
#include "Application.h"

// Number of swap chain back buffers
constexpr uint8_t g_NumFrames = 3;

std::unique_ptr<DDM::Application> g_pApplication;

// Set to true once the DX12 objects have been initialized
bool g_IsInitialized = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_IsInitialized)
    {
        g_pApplication->WndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return ::DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


int CALLBACK WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{ 
    g_pApplication = std::make_unique<DDM::Application>(&WndProc, hInstance, L"Learning DirectX 12", g_NumFrames);

    g_IsInitialized = true;
    
    g_pApplication->Run();

    return 0;
}