// Applicatoin.cpp

// Header include
#include "Application.h"

// File includes
#include "Window.h" // For DDM::Window class
#include "FenceObject.h" // For DDM::FenceObjct class
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"


DDM::Application::Application()
{
}

DDM::Application::~Application()
{
    // Make sure the command queue has finished all commands before closing.
    g_pFenceObject->CloseHandle(g_CommandQueue);
}

bool DDM::Application::Initialize(WNDPROC pWndProc, HINSTANCE hInst, const wchar_t* windowTitle, uint8_t numFrames)
{
    g_NumFrames = numFrames;
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Usint this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    ParseCommandLineArguments();

    EnableDebugLayer();

    g_Device = CreateDevice(GetAdapter(m_UseWarp));

    m_pWindow = std::make_unique<DDM::Window>(pWndProc, hInst, windowTitle, numFrames);

    g_CommandQueue = CreateCommandQueue(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_pWindow->CreateSwapchain(g_CommandQueue, g_Device);

    g_CommandAllocators.resize(g_NumFrames);
    for (int i = 0; i < g_NumFrames; ++i)
    {
        g_CommandAllocators[i] = CreateCommandAllocator(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    }

    g_CommandList = CreateCommandList(g_Device, g_CommandAllocators[m_pWindow->GetCurrentBackBufferIndex()], D3D12_COMMAND_LIST_TYPE_DIRECT);

    g_pFenceObject = std::make_unique<DDM::FenceObject>(g_Device, g_NumFrames);
    g_FrameFenceValues.resize(g_NumFrames);

    m_pWindow->ShowWindow();

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

void DDM::Application::GameLoop()
{
    Update();
    Render();
}

void DDM::Application::Update()
{
    static uint64_t frameCounter = 0;
    static double elapsedSeconds = 0.0;
    static std::chrono::high_resolution_clock clock;
    static auto t0 = clock.now();

    frameCounter++;
    auto t1 = clock.now();
    auto deltaTime = t1 - t0;
    t0 = t1;
    elapsedSeconds += deltaTime.count() * 1e-9;
    if (elapsedSeconds > 1.0)
    {
        char buffer[500];
        auto fps = frameCounter / elapsedSeconds;
        sprintf_s(buffer, 500, "FPS: %f\n", fps);
        OutputDebugString(buffer);

        frameCounter = 0;
        elapsedSeconds = 0.0;
    }
}

void DDM::Application::Render()
{
    auto& commandAllocator = g_CommandAllocators[m_pWindow->GetCurrentBackBufferIndex()];
    auto& backBuffer = m_pWindow->GetCurrentBackBuffer();

    commandAllocator->Reset();
    g_CommandList->Reset(commandAllocator.Get(), nullptr);

    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        g_CommandList->ResourceBarrier(1, &barrier);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetRTV();

        g_CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // Present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        g_CommandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(g_CommandList->Close());


        ID3D12CommandList* const commandLists[] =
        {
              g_CommandList.Get()
        };
        g_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);


        m_pWindow->PresentSwapchain();

        g_FrameFenceValues[m_pWindow->GetCurrentBackBufferIndex()] = g_pFenceObject->Signal(g_CommandQueue);

        m_pWindow->SetCurrentBackBufferIndex();

        g_pFenceObject->WaitForFenceValue(g_FrameFenceValues[m_pWindow->GetCurrentBackBufferIndex()]);
    }
}

LRESULT DDM::Application::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        GameLoop();
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        switch (wParam)
        {
        case 'V':
            m_pWindow->ToggleVsync();
            break;
        case VK_ESCAPE:
            ::PostQuitMessage(0);
            break;
        case VK_RETURN:
            if (alt)
            {
                m_pWindow->ToggleFullscreen();
            }
            break;
        case VK_F11:
            m_pWindow->ToggleFullscreen();
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

        ::GetClientRect(m_pWindow->GetWindowHandle(), &clientRect);

        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;

        m_pWindow->Resize(width, height, g_Device, g_CommandQueue, g_pFenceObject.get(), g_FrameFenceValues);
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