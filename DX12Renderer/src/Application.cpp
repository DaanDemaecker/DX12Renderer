// Applicatoin.cpp

// Header include
#include "Application.h"

// File includes
#include "Window.h" // For DDM::Window class
#include "FenceObject.h" // For DDM::FenceObjct class
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"
#include "Events.h"
#include "CommandQueue.h"

static std::shared_ptr<DDM::Window> gs_Window;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


DDM::Application::Application()
{

}

DDM::Application::~Application()
{
    // Make sure the command queue has finished all commands before closing.
    //g_pFenceObject->CloseHandle(g_CommandQueue);
}

bool DDM::Application::Initialize(HINSTANCE hInst, uint8_t numFrames)
{
    g_NumFrames = numFrames;
    
    m_Instance = hInst;

    RegisterWindowClass(hInst, m_WindowClassName);
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Usint this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    ParseCommandLineArguments();

    EnableDebugLayer();

    g_Device = CreateDevice(GetAdapter(m_UseWarp));
    
    m_pCommandQueue = std::make_unique<DDM::CommandQueue>(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    
    g_pFenceObject = std::make_unique<DDM::FenceObject>(g_Device, g_NumFrames);
    g_FrameFenceValues.resize(g_NumFrames);

    return true;
}

void DDM::Application::Run()
{
    MSG msg = {};

    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    DestroyWindow();
}

void DDM::Application::ParseCommandLineArguments()
{
    int argc;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    for (size_t i = 0; i < argc; ++i)
    {
        if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
        {
            m_UseWarp = true;
        }
    }

    // Free memory allocated by CommandLineToArgvW
    ::LocalFree(argv);
}


void DDM::Application::RegisterWindowClass(HINSTANCE hInst, const std::wstring& windowClassName)
{
    // Register a window class for creating our render window with.
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInst;
    windowClass.hIcon = NULL; //LoadIcon(hInst, NULL);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName.c_str();
    windowClass.hIconSm = NULL; //LoadIcon(hInst, NULL);

    static ATOM atom = ::RegisterClassExW(&windowClass);
    assert(atom > 0);
}

void DDM::Application::DestroyWindow()
{
    gs_Window = nullptr;
}

std::shared_ptr<DDM::Window> DDM::Application::CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vsync)
{
    gs_Window = std::make_shared<DDM::Window>(m_WindowClassName, m_Instance, windowName, g_NumFrames, clientWidth, clientHeight, vsync);

    gs_Window->CreateSwapchain(Application::Get().GetCommandQueue()->GetD3D12CommandQueue(), g_Device);

    gs_Window->ShowWindow();

    return gs_Window;
}

DDM::CommandQueue* DDM::Application::GetCommandQueue()
{
    return m_pCommandQueue.get();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        RenderEventArgs renderArgs{ 0, 0 };
        gs_Window->OnRender(renderArgs);
    }
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        switch (wParam)
        {
        case 'V':
            gs_Window->ToggleVsync();
            break;
        case VK_ESCAPE:
            ::PostQuitMessage(0);
            break;
        case VK_RETURN:
            if (alt)
            {
                gs_Window->ToggleFullscreen();
            }
            break;
        case VK_F11:
            gs_Window->ToggleFullscreen();
            break;
        }
        break;
    }

    // The default window procedure will play a system notification sound
    // when pressing the Alt + Enter keyboard combination if this message is not handled

    case WM_SYSCHAR:
        break;

    case WM_SIZE:
    {
        RECT clientRect = {};

        ::GetClientRect(gs_Window->GetWindowHandle(), &clientRect);

        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        //gs_Windows->Resize(width, height, g_Device, g_CommandQueue, g_pFenceObject.get(), g_FrameFenceValues);
    }
    break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    default:
        return ::DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}