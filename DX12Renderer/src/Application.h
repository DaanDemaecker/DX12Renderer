// Application.h

#ifndef ApplicationIncluded
#define ApplicationIncluded

// Standard library includes
#include <memory> // For std::unique_ptr
#include <Windows.h>
#include <inttypes.h>

namespace DDM
{
	class Window;

	class Application final
	{
	public:
		Application(WNDPROC pWndProc, HINSTANCE hInst,
			const wchar_t* windowTitle, uint8_t numFrames);

		~Application();

		Application(Application& other) = delete;
		Application(Application&& other) = delete;

		Application& operator=(Application& other) = delete;
		Application& operator=(Application&& other) = delete;

		Window* GetWindow();

	private:
		std::unique_ptr<Window> m_pWindow{};

	};
}

#endif // !ApplicationIncluded
