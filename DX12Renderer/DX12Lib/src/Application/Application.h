// Application.h

#ifndef ApplicationIncluded
#define ApplicationIncluded

// File includes
#include "../Includes/DirectXIncludes.h"
#include "Singleton.h"
#include "CommandQueue.h"

// Standard library includes
#include <memory> // For std::unique_ptr
#include <Windows.h>
#include <inttypes.h>
#include <vector>
#include <wrl.h> // For Microsoft::WRL::Comptr
#include <string>
using namespace Microsoft::WRL;

namespace DDM
{
	class Window;
	class Game;

	class Application final : public Singleton<Application>
	{
	public:
		Application();

		~Application();

		Application(Application& other) = delete;
		Application(Application&& other) = delete;

		Application& operator=(Application& other) = delete;
		Application& operator=(Application&& other) = delete;
		
		bool Initialize(HINSTANCE hIns);

		void ShutDown();

		int Run(std::shared_ptr<Game> pGame);

		std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vsync = true);

		CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type);

		ComPtr<ID3D12Device5> GetDevice();

		void Flush();

		UINT FrameCount() const { return m_FrameCount; }

		uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);
		
		void QueryRaytracingSupport();
	private:
		// Number of frames in flight.
		const UINT m_FrameCount = 3;

		std::wstring m_WindowClassName = L"DX12WindowClass";
		
		HINSTANCE m_Instance = nullptr;

		// Use WARP adapter
		bool m_UseWarp = false;

		// DirectX 12 Objects
		ComPtr<ID3D12Device5> m_Device;

		std::unique_ptr<CommandQueue> m_pDirectCommandQueue;
		std::unique_ptr<CommandQueue> m_pCopyCommandQueue;

		void ParseCommandLineArguments();

		void RegisterWindowClass(HINSTANCE hInst, const std::wstring& windowClassName);

		void DestroyWindow();
	};
}

#endif // !ApplicationIncluded
