// Applicatoin.cpp

// Header include
#include "Application.h"

// File includes
#include "Window.h" // For DDM::Window class
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"
#include "Events.h"
#include "CommandQueue.h"
#include "Games/Game.h"

static std::shared_ptr<DDM::Window> gs_Window;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


DDM::Application::Application()
{

}

DDM::Application::~Application()
{
    // Make sure the command queue has finished all commands before closing.
    //g_pFenceObject->CloseHandle(g_CommandQueue);

    ComPtr<IDXGIDebug1> dxgiDebug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
    {
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    }
}

bool DDM::Application::Initialize(HINSTANCE hInst)
{
    m_Instance = hInst;

    RegisterWindowClass(hInst, m_WindowClassName);
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Usint this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    ParseCommandLineArguments();

    EnableDebugLayer();

    m_Device = CreateDevice(GetAdapter(m_UseWarp));
    
    m_pDirectCommandQueue = std::make_unique<DDM::CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_pCopyCommandQueue = std::make_unique<DDM::CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);

    return true;
}

void DDM::Application::ShutDown()
{
    DestroyWindow();
    m_Device.Reset();
    m_pDirectCommandQueue.reset();
    m_pCopyCommandQueue.reset();

}

int DDM::Application::Run(std::shared_ptr<Game> pGame)
{
    if (!pGame->Initialize()) return 1;
    if (!pGame->LoadContent()) return 2;


    MSG msg = {};

    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    m_pDirectCommandQueue->Flush();
    m_pCopyCommandQueue->Flush();

    pGame->UnloadContent();
    pGame->Destroy();

    return static_cast<int>(msg.wParam);
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
    if (gs_Window != nullptr)
    {
        gs_Window->ClearGame();
    }

    gs_Window = nullptr;
}

std::shared_ptr<DDM::Window> DDM::Application::CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vsync)
{
    gs_Window = std::make_shared<DDM::Window>(m_Device, m_WindowClassName, m_Instance, windowName, clientWidth, clientHeight, vsync, m_FrameCount);

    gs_Window->CreateSwapchain(Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetD3D12CommandQueue(), m_Device);

    gs_Window->ShowWindow();

    return gs_Window;
}

DDM::CommandQueue* DDM::Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return m_pDirectCommandQueue.get();
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return m_pCopyCommandQueue.get();
        break;
    }


    return nullptr;
}

ComPtr<ID3D12Device5> DDM::Application::GetDevice()
{
    return m_Device;
}

void DDM::Application::Flush()
{
    m_pDirectCommandQueue->Flush();
    m_pCopyCommandQueue->Flush();
}

uint32_t DDM::Application::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    return m_Device->GetDescriptorHandleIncrementSize(type);
}

void DDM::Application::QueryRaytracingSupport()
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    ThrowIfFailed(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
        &options5, sizeof(options5)));
    if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
        throw std::runtime_error("Raytracing not supported on device");
}

// Convert the message ID into a MouseButton ID
static MouseButtonEventArgs::MouseButton DecodeMouseButton(UINT messageID)
{
    MouseButtonEventArgs::MouseButton mouseButton = MouseButtonEventArgs::None;
    switch (messageID)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Left;
    }
    break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Right;
    }
    break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    {
        mouseButton = MouseButtonEventArgs::Middel;
    }
    break;
    }

    return mouseButton;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   // if (gs_Window == nullptr)
   //     return 0;

    switch (message)
    {
    case WM_PAINT:
    {
        // Delta time will be filled in by the Window.
        UpdateEventArgs updateEventArgs(0.0f, 0.0f);
        gs_Window->OnUpdate(updateEventArgs);
        RenderEventArgs renderEventArgs(0.0f, 0.0f);
        // Delta time will be filled in by the Window.
        gs_Window->OnRender(renderEventArgs);
    }
    break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        MSG charMsg;
        // Get the Unicode character (UTF-16)
        unsigned int c = 0;
        // For printable characters, the next message will be WM_CHAR.
        // This message contains the character code we need to send the KeyPressed event.
        // Inspired by the SDL 1.2 implementation.
        if (PeekMessage(&charMsg, hwnd, 0, 0, PM_NOREMOVE) && charMsg.message == WM_CHAR)
        {
            GetMessage(&charMsg, hwnd, 0, 0);
            c = static_cast<unsigned int>(charMsg.wParam);
        }
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        KeyCode::Key key = (KeyCode::Key)wParam;
        unsigned int scanCode = (lParam & 0x00FF0000) >> 16;
        KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Pressed, shift, control, alt);
        gs_Window->OnKeyPressed(keyEventArgs);
    }
    break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        KeyCode::Key key = (KeyCode::Key)wParam;
        unsigned int c = 0;
        unsigned int scanCode = (lParam & 0x00FF0000) >> 16;

        // Determine which key was released by converting the key code and the scan code
        // to a printable character (if possible).
        // Inspired by the SDL 1.2 implementation.
        unsigned char keyboardState[256];
        GetKeyboardState(keyboardState);
        wchar_t translatedCharacters[4];
        if (int result = ToUnicodeEx(static_cast<UINT>(wParam), scanCode, keyboardState, translatedCharacters, 4, 0, NULL) > 0)
        {
            c = translatedCharacters[0];
        }

        KeyEventArgs keyEventArgs(key, c, KeyEventArgs::Released, shift, control, alt);
        gs_Window->OnKeyReleased(keyEventArgs);
    }
    break;
    // The default window procedure will play a system notification sound 
    // when pressing the Alt+Enter keyboard combination if this message is 
    // not handled.
    case WM_SYSCHAR:
        break;
    case WM_MOUSEMOVE:
    {
        bool lButton = (wParam & MK_LBUTTON) != 0;
        bool rButton = (wParam & MK_RBUTTON) != 0;
        bool mButton = (wParam & MK_MBUTTON) != 0;
        bool shift = (wParam & MK_SHIFT) != 0;
        bool control = (wParam & MK_CONTROL) != 0;

        int x = ((int)(short)LOWORD(lParam));
        int y = ((int)(short)HIWORD(lParam));

        MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
        gs_Window->OnMouseMoved(mouseMotionEventArgs);
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
        bool lButton = (wParam & MK_LBUTTON) != 0;
        bool rButton = (wParam & MK_RBUTTON) != 0;
        bool mButton = (wParam & MK_MBUTTON) != 0;
        bool shift = (wParam & MK_SHIFT) != 0;
        bool control = (wParam & MK_CONTROL) != 0;

        int x = ((int)(short)LOWORD(lParam));
        int y = ((int)(short)HIWORD(lParam));

        MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y);
        gs_Window->OnMouseButtonPressed(mouseButtonEventArgs);
    }
    break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        bool lButton = (wParam & MK_LBUTTON) != 0;
        bool rButton = (wParam & MK_RBUTTON) != 0;
        bool mButton = (wParam & MK_MBUTTON) != 0;
        bool shift = (wParam & MK_SHIFT) != 0;
        bool control = (wParam & MK_CONTROL) != 0;

        int x = ((int)(short)LOWORD(lParam));
        int y = ((int)(short)HIWORD(lParam));

        MouseButtonEventArgs mouseButtonEventArgs(DecodeMouseButton(message), MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y);
        gs_Window->OnMouseButtonReleased(mouseButtonEventArgs);
    }
    break;
    case WM_MOUSEWHEEL:
    {
        // The distance the mouse wheel is rotated.
        // A positive value indicates the wheel was rotated to the right.
        // A negative value indicates the wheel was rotated to the left.
        float zDelta = ((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA;
        short keyStates = (short)LOWORD(wParam);

        bool lButton = (keyStates & MK_LBUTTON) != 0;
        bool rButton = (keyStates & MK_RBUTTON) != 0;
        bool mButton = (keyStates & MK_MBUTTON) != 0;
        bool shift = (keyStates & MK_SHIFT) != 0;
        bool control = (keyStates & MK_CONTROL) != 0;

        int x = ((int)(short)LOWORD(lParam));
        int y = ((int)(short)HIWORD(lParam));

        // Convert the screen coordinates to client coordinates.
        POINT clientToScreenPoint;
        clientToScreenPoint.x = x;
        clientToScreenPoint.y = y;
        ScreenToClient(hwnd, &clientToScreenPoint);

        MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, (int)clientToScreenPoint.x, (int)clientToScreenPoint.y);
        gs_Window->OnMouseWheel(mouseWheelEventArgs);
    }
    break;
    case WM_SIZE:
    {
        int width = ((int)(short)LOWORD(lParam));
        int height = ((int)(short)HIWORD(lParam));

        ResizeEventArgs resizeEventArgs(width, height);
        gs_Window->OnResize(resizeEventArgs);
    }
    break;
    case WM_DESTROY:
    {
        // If there are no more windows, quit the application.
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}