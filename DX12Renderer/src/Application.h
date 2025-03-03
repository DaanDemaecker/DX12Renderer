// Application.h

#ifndef ApplicationIncluded
#define ApplicationIncluded

// File includes
#include "Includes/DirectXIncludes.h"
#include "Singleton.h"

// Standard library includes
#include <memory> // For std::unique_ptr
#include <Windows.h>
#include <inttypes.h>
#include <vector>
#include <wrl.h> // For Microsoft::WRL::Comptr
using namespace Microsoft::WRL;

namespace DDM
{
	class Window;
	class FenceObject;

	class Application final : public Singleton<Application>
	{
	public:
		Application();

		~Application();

		Application(Application& other) = delete;
		Application(Application&& other) = delete;

		Application& operator=(Application& other) = delete;
		Application& operator=(Application&& other) = delete;
		
		bool Initialize(WNDPROC pWndProc, HINSTANCE hInst, const wchar_t* windowTitle, uint8_t numFrames);

		void Run();

		LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:

		// Use WARP adapter
		bool m_UseWarp = false;

		std::unique_ptr<Window> m_pWindow{};

		uint8_t g_NumFrames;

		// DirectX 12 Objects
		ComPtr<ID3D12Device2> g_Device;
		ComPtr<ID3D12CommandQueue> g_CommandQueue;
		ComPtr<ID3D12GraphicsCommandList> g_CommandList;
		std::vector<ComPtr<ID3D12CommandAllocator>> g_CommandAllocators;

		// Fence objects
		std::unique_ptr<DDM::FenceObject> g_pFenceObject;
		std::vector<uint64_t> g_FrameFenceValues;

		void ParseCommandLineArguments();

		void GameLoop();

		void Update();

		void Render();
	};
}

#endif // !ApplicationIncluded
