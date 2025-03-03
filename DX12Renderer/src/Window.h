#ifndef WindowIncluded
#define WindowIncluded

#define WIN_32_LEAN_AND_MEAN

// File includes
#include "Includes/DirectXIncludes.h"

// Standard library includes
#include <inttypes.h> // For uint32_t
#include <Windows.h>
#include <wrl.h> // For Microsoft::WRL::Comptr
using namespace Microsoft::WRL;
#include <vector>

// In order to define a function called CreateWindow, the Windows macro needs to be undifined
#if defined(CreateWindow)
#undef CreateWindow
#endif

namespace DDM
{
	class FenceObject;

	class Window
	{
	public:
		Window() = delete;
		Window(const std::wstring& windowClassName, HINSTANCE hInst,
			const std::wstring& windowTitle, uint8_t numFrames,
			int clientWidth, int clientHeight, bool vsync);
		
		~Window();

		Window(Window& other) = delete;
		Window(Window&& other) = delete;

		Window& operator=(Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		void ToggleFullscreen();
		void SetFullscreen(bool fullscreen);

		HWND GetWindowHandle() { return m_hWnd; }

		void Resize(uint32_t width, uint32_t height, ComPtr<ID3D12Device2> device,
			ComPtr<ID3D12CommandQueue> commandQueue, FenceObject* pFenceObject, std::vector<uint64_t>& frameFenceValues);

		void CreateSwapchain(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Device2> device);

		UINT GetCurrentBackBufferIndex() const { return m_CurrentBackBufferIndex; }

		void SetCurrentBackBufferIndex();

		ComPtr<ID3D12Resource>& GetCurrentBackBuffer() { return m_BackBuffers[m_CurrentBackBufferIndex]; }

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV();

		void PresentSwapchain();

		void ToggleVsync() { m_VSync = !m_VSync; }

		void ShowWindow();

		void OnRender();


	private:
		// The Window procedure needs to call protected methods of this class.
		friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

		// Window Handle
		HWND m_hWnd;
		// Window rectangle  (used to toggle fullscreen state)
		RECT m_WindowRect;

		// Amount of frames in flight
		uint8_t m_NumFrames;

		// Client dimensions
		int m_ClientWidth;
		int m_ClientHeight;

		// By default, enable V-Sync.
		// Can be toggled with the V key.
		bool m_VSync = true;
		bool m_TearingSupported = false;

		// By default, use windowed mode.
		// Can be toggled with the Alt+Enter or F11
		bool m_Fullscreen = false;

		// Swapchain variables
		ComPtr<IDXGISwapChain4> m_SwapChain;
		std::vector<ComPtr<ID3D12Resource>> m_BackBuffers;
		UINT m_CurrentBackBufferIndex;
		ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
		UINT m_RTVDescriptorSize;

		void ParseCommandLineArgs();

		void RegisterWindowClass(WNDPROC pWndProc, HINSTANCE hInst, const wchar_t* windowClassName);

		HWND CreateWindow(const std::wstring& windowClassName, HINSTANCE hInst,
			const wchar_t* windowTitle, uint32_t width, uint32_t height);

		ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd,
			ComPtr<ID3D12CommandQueue> commandQueue,
			uint32_t width, uint32_t height, uint32_t bufferCount);

		void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device,
			ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
	};
}

#endif // !WindowIncluded
