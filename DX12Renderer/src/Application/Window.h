#ifndef WindowIncluded
#define WindowIncluded

#define WIN_32_LEAN_AND_MEAN

// File includes
#include "Includes/DirectXIncludes.h"
#include "Events.h"

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
	class Game;
	class HighResClock;

	class Window
	{
	public:
		// Number of swapchain back buffers.
		static const UINT BufferCount = 3;

		Window() = delete;
		Window(ComPtr<ID3D12Device2> device, const std::wstring& windowClassName, HINSTANCE hInst,
			const std::wstring& windowTitle,
			int clientWidth, int clientHeight, bool vsync);
		
		~Window();

		Window(Window& other) = delete;
		Window(Window&& other) = delete;

		Window& operator=(Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		void ToggleFullscreen();
		void SetFullscreen(bool fullscreen);

		HWND GetWindowHandle() { return m_hWnd; }

		void Resize(uint32_t width, uint32_t height);

		void CreateSwapchain(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Device2> device);

		UINT GetCurrentBackBufferIndex() const { return m_CurrentBackBufferIndex; }

		void SetCurrentBackBufferIndex();

		ComPtr<ID3D12Resource>& GetCurrentBackBuffer() { return m_BackBuffers[m_CurrentBackBufferIndex]; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV();

		void PresentSwapchain();

		void ToggleVsync() { m_VSync = !m_VSync; }

		void ShowWindow();

		void RegisterGame(std::shared_ptr<Game> pGame);

		void UnRegisterGame(std::shared_ptr<Game> pGame);

		/**
		* Update the game logic
		*/
		void OnUpdate(UpdateEventArgs& e);

		/**
		* Render stuff
		*/
		void OnRender(RenderEventArgs& e);

		/**
		* Invoked by the registered window when a key is pressed
		* while window has focus
		*/
		void OnKeyPressed(KeyEventArgs& e);

		/**
		* Invoked when a key on the keyboard is released
		* when the window has focus
		*/
		void OnKeyReleased(KeyEventArgs& e);

		/**
		* Invoked when the mouse is moved over the registered window
		*/
		void OnMouseMoved(MouseMotionEventArgs& e);

		/**
		* Invoked when a mouse buttons is pressed over the registered window
		*/
		void OnMouseButtonPressed(MouseButtonEventArgs& e);

		/**
		* Invoked when mouse button is released over the registered window
		*/
		void OnMouseButtonReleased(MouseButtonEventArgs& e);

		/**
		* Invoked when the mouse wheel is scrolled while the registered window has focus
		*/
		void OnMouseWheel(MouseWheelEventArgs& e);

		/**
		* Invoked when the attached window is resized
		*/
		 void OnResize(ResizeEventArgs& e);;

		UINT Present();

	private:
		// Window Handle
		HWND m_hWnd;
		// Window rectangle  (used to toggle fullscreen state)
		RECT m_WindowRect;

		ComPtr<ID3D12Device2> m_Device;

		// The registered game class for this window
		std::shared_ptr<Game> m_pGame{};

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

		// Clocks
		std::unique_ptr<HighResClock> m_pUpdateClock;
		std::unique_ptr<HighResClock> m_pRenderClock;

		void ParseCommandLineArgs();

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
