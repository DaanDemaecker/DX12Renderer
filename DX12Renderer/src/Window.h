#ifndef WindowIncluded
#define WindowIncluded

#define WIN_32_LEAN_AND_MEAN

// File includes
#include "Includes/DirectXIncludes.h"

// Standard library includes
#include <inttypes.h> // For uint32_t
#include <Windows.h>

// In order to define a function called CreateWindow, the Windows macro needs to be undifined
#if defined(CreateWindow)
#undef CreateWindow
#endif

namespace DDM
{
	class Window
	{
	public:
		Window() = delete;
		Window(WNDPROC pWndProc, HINSTANCE hInst,
			const wchar_t* windowTitle, uint8_t numFrames);
		
		~Window();

		Window(Window& other) = delete;
		Window(Window&& other) = delete;

		Window& operator=(Window& other) = delete;
		Window& operator=(Window&& other) = delete;

		uint32_t GetClientWidth() const { return m_ClientWidth; }
		uint32_t GetClientHeight() const { return m_ClientHeight; }

		void SetClientDimensions(uint32_t width, uint32_t height);

		void ToggleFullscreen();
		void SetFullscreen(bool fullscreen);

		HWND GetWindowHandle() { return m_hWnd; }
	private:
		// Window Handle
		HWND m_hWnd;
		// Window rectangle  (used to toggle fullscreen state)
		RECT m_WindowRect;

		uint8_t m_NumFrames;

		uint32_t m_ClientWidth = 1280;
		uint32_t m_ClientHeight = 720;

		// By default, use windowed mode.
		// Can be toggled with the Alt+Enter or F11
		bool m_Fullscreen = false;

		void ParseCommandLineArgs();

		void RegisterWindowClass(WNDPROC pWndProc, HINSTANCE hInst, const wchar_t* windowClassName);

		HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst,
			const wchar_t* windowTitle, uint32_t width, uint32_t height);

	};
}

#endif // !WindowIncluded
