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
	class CommandQueue;

	class Application final : public Singleton<Application>
	{
	public:
		Application();

		~Application();

		Application(Application& other) = delete;
		Application(Application&& other) = delete;

		Application& operator=(Application& other) = delete;
		Application& operator=(Application&& other) = delete;
		
		bool Initialize(HINSTANCE hIns, uint8_t numFrames);

		void Run();

		std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vsync = true);

		CommandQueue* GetCommandQueue();
	private:
		std::wstring m_WindowClassName = L"DX12WindowClass";
		
		HINSTANCE m_Instance;

		// Use WARP adapter
		bool m_UseWarp = false;

		uint8_t g_NumFrames;

		// DirectX 12 Objects
		ComPtr<ID3D12Device2> g_Device;

		std::unique_ptr<CommandQueue> m_pCommandQueue;

		ComPtr<ID3D12CommandQueue> g_CommandQueue;
		ComPtr<ID3D12GraphicsCommandList> g_CommandList;
		std::vector<ComPtr<ID3D12CommandAllocator>> g_CommandAllocators;

		// Fence objects
		std::unique_ptr<DDM::FenceObject> g_pFenceObject;
		std::vector<uint64_t> g_FrameFenceValues;

		void ParseCommandLineArguments();

		void RegisterWindowClass(HINSTANCE hInst, const std::wstring& windowClassName);

		void DestroyWindow();
	};
}

#endif // !ApplicationIncluded
