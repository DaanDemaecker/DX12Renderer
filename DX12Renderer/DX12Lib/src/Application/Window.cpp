// Window.cpp
#include "Window.h"

// File includes
#include "Helpers/Helpers.h"
#include "Helpers/DirectXHelpers.h"
#include "CommandQueue.h"
#include "Application.h"
#include "Games/Game.h"
#include "HighResClock.h"

// Standard library includes
#include <shellapi.h> // For CommandLineToArgvW
#include <cassert>
#include <algorithm>


DDM::Window::Window(ComPtr<ID3D12Device2> device, const std::wstring& windowClassName, HINSTANCE hInst, const std::wstring& windowTitle,
    int clientWidth, int clientHeight, bool vsync)
    : m_Device{device}, m_ClientWidth{clientWidth}, m_ClientHeight{clientHeight}, m_VSync{vsync}
{
    ParseCommandLineArgs();

    m_hWnd = CreateWindow(windowClassName, hInst, windowTitle.c_str(), m_ClientWidth, m_ClientHeight);


    // Initialize the global window rect variable.
    ::GetWindowRect(m_hWnd, &m_WindowRect);

    m_TearingSupported = CheckTearingSupport();

    m_pUpdateClock = std::make_unique<DDM::HighResClock>();
    m_pRenderClock = std::make_unique<DDM::HighResClock>();
}

DDM::Window::~Window()
{

}

void DDM::Window::Resize(uint32_t width, uint32_t height)
{
    if (m_ClientWidth != width || m_ClientHeight != height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_ClientWidth = (std::max)(1u, width);
        m_ClientHeight = (std::max)(1u, height);

        // Flush the GPU queue to make sure the swap chain's back buffers
        // are not being referenced by an in-flight command list.
        Application::Get().Flush();
        

        for (int i = 0; i < BufferCount; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            m_BackBuffers[i].Reset();
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
        ThrowIfFailed(m_SwapChain->ResizeBuffers(BufferCount, m_ClientWidth, m_ClientHeight,
            swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews(m_Device, m_SwapChain, m_RTVDescriptorHeap);
    }
}

void DDM::Window::CreateSwapchain(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Device2> device)
{
    m_BackBuffers.resize(BufferCount);

    m_SwapChain = CreateSwapChain(m_hWnd, commandQueue,
        m_ClientWidth, m_ClientHeight, BufferCount);

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    m_RTVDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, BufferCount);
    m_RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    UpdateRenderTargetViews(device, m_SwapChain, m_RTVDescriptorHeap);
}

void DDM::Window::SetCurrentBackBufferIndex()
{
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

D3D12_CPU_DESCRIPTOR_HANDLE DDM::Window::GetCurrentRenderTargetView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrentBackBufferIndex, m_RTVDescriptorSize);

}

CD3DX12_CPU_DESCRIPTOR_HANDLE DDM::Window::GetRTV()
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrentBackBufferIndex, m_RTVDescriptorSize);
}

void DDM::Window::PresentSwapchain()
{
    UINT syncInterval = m_VSync ? 1 : 0;
    UINT presentFlags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_SwapChain->Present(syncInterval, presentFlags));
}

void DDM::Window::ShowWindow()
{
    ::ShowWindow(m_hWnd, SW_SHOW);
}

void DDM::Window::RegisterGame(std::shared_ptr<Game> pGame)
{
    m_pGame = pGame;
}

void DDM::Window::UnRegisterGame(std::shared_ptr<Game> pGame)
{
    if (pGame == m_pGame)
    {
        pGame = nullptr;
    }
}

void DDM::Window::OnUpdate(UpdateEventArgs&)
{
    m_pUpdateClock->Tick();

    if (m_pGame)
    {
        UpdateEventArgs updateEventArgs(m_pUpdateClock->GetElapsedSec(), m_pUpdateClock->GetTotalTime());
        m_pGame->OnUpdate(updateEventArgs);
    }
}

void DDM::Window::OnRender(RenderEventArgs&)
{
    m_pRenderClock->Tick();

    if (m_pGame)
    {
        RenderEventArgs renderEventArgs(m_pRenderClock->GetElapsedSec(), m_pRenderClock->GetTotalTime());
        m_pGame->OnRender(renderEventArgs);
    }
}

void DDM::Window::OnKeyPressed(KeyEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnKeyPressed(e);
    }
}

void DDM::Window::OnKeyReleased(KeyEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnKeyReleased(e);
    }
}

void DDM::Window::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnMouseMoved(e);
    }
}

void DDM::Window::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnMouseButtonPressed(e);
    }
}

void DDM::Window::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnMouseButtonReleased(e);
    }
}

void DDM::Window::OnMouseWheel(MouseWheelEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnMouseWheel(e);
    }
}

void DDM::Window::OnResize(ResizeEventArgs& e)
{
    if (m_pGame)
    {
        m_pGame->OnResize(e);
    }

    Resize(e.Width, e.Height);
}

void DDM::Window::ParseCommandLineArgs()
{
    int argc;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    for (size_t i = 0; i < argc; ++i)
    {
        if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
        {
            m_ClientWidth = ::wcstol(argv[++i], nullptr, 10);
        }
        if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
        {
            m_ClientHeight = ::wcstol(argv[++i], nullptr, 10);
        }
    }

    // Free memory allocated by CommandLineToArgvW
    ::LocalFree(argv);
}

HWND DDM::Window::CreateWindow(const std::wstring& windowClassName, HINSTANCE hInst,
    const wchar_t* windowTitle, uint32_t width, uint32_t height)
{
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    HWND hWnd = ::CreateWindowExW(
        NULL,
        windowClassName.c_str(),
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        hInst,
        nullptr
    );

    assert(hWnd && "Failed to create window");


    return hWnd;
}

void DDM::Window::ToggleFullscreen()
{
    SetFullscreen(!m_Fullscreen);
}

void DDM::Window::SetFullscreen(bool fullscreen)
{
    if (m_Fullscreen != fullscreen)
    {
        m_Fullscreen = fullscreen;

        if (m_Fullscreen) // Switching to fullscreen
        {
            // Store the current window dimensions so they can be restored
            // when switching out of fullscreen state.

            ::GetWindowRect(m_hWnd, &m_WindowRect);

            // Set the window style to a borderless window so the client area fills
            // the entire screen
            UINT windowStyle = WS_OVERLAPPEDWINDOW &
                ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

            ::SetWindowLongW(m_hWnd, GWL_STYLE, windowStyle);

            // Query the name of the nearest display device for the window.
            // This is required to set the fullscreen dimensions of the window
            // when using a multi-monitor setup.
            HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorInfo = {};
            monitorInfo.cbSize = sizeof(MONITORINFOEX);

            ::GetMonitorInfo(hMonitor, &monitorInfo);

            ::SetWindowPos(m_hWnd, HWND_TOP,
                monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWnd, SW_MAXIMIZE);
        }
        else // Restore all the window decorators
        {
            ::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

            ::SetWindowPos(m_hWnd, HWND_NOTOPMOST,
                m_WindowRect.left, m_WindowRect.top,
                m_WindowRect.right - m_WindowRect.left,
                m_WindowRect.bottom - m_WindowRect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE);

            ::ShowWindow(m_hWnd, SW_NORMAL);
        }
    }
}

ComPtr<IDXGISwapChain4> DDM::Window::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

    return dxgiSwapChain4;
}

void DDM::Window::UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < BufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        m_BackBuffers[i] = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

UINT DDM::Window::Present()
{
    UINT syncInterval = m_VSync ? 1 : 0;
    UINT presentFlags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(m_SwapChain->Present(syncInterval, presentFlags));
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

    return m_CurrentBackBufferIndex;
}

void DDM::Window::ClearGame()
{
    m_pGame = nullptr;
}



