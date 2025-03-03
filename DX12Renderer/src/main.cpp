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


int CALLBACK WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{ 
    DDM::Application::Get().Initialize(hInstance, g_NumFrames);

    DDM::Application::Get().CreateRenderWindow(L"DirectX12 Tutorial", 1280, 720, true);
    
    DDM::Application::Get().Run();

    return 0;
}