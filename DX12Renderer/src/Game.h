// Game.h
// This class will serve as the base for any game class the user creates

#ifndef GameIncluded
#define GameIncluded

// File includes
#include "Events.h"

#include <memory> // for std::enabled_shared_from_this
#include <string> // for std::wstring

namespace DDM
{
	class Window;

	class Game : public std::enable_shared_from_this<Game>
	{
	public:
		/**
		* Create the DirectX demo using the specified window dimensions.
		*/
		Game(const std::wstring& name, int width, int height, bool vSync);
		virtual ~Game();

		// Delete default constructor
		Game() = delete;

		// Delete copy and move operators
		Game(Game& other) = delete;
		Game(Game&& other) = delete;

		Game& operator=(Game& other) = delete;
		Game& operator=(Game&& other) = delete;

		int GetClientWidth() const
		{
			return m_Width;
		}

		int GetClientHeight() const
		{
			return m_Height;
		}

		/**
		* Initialze the DirectX Runtime.
		*/
		virtual bool Initialize();

		/**
		* Load content required for the game.
		*/
		virtual bool LoadContent() = 0;

		/**
		* Unload game specific content that was loaded in LoadContent.
		*/
		virtual void UnloadContent() = 0;

		/**
		* Destroy any resources that are used by the game
		*/
		virtual void Destroy();

	protected:
		friend class Window;

		/**
		* Update the game logic
		*/
		virtual void OnUpdate(UpdateEventArgs& e);

		/**
		* Render stuff
		*/
		virtual void OnRender(RenderEventArgs& e);

		/**
		* Invoked by the registered window when a key is pressed
		* while window has focus
		*/
		virtual void OnKeyPressed(KeyEventArgs& e);

		/**
		* Invoked when a key on the keyboard is released
		* when the window has focus
		*/
		virtual void OnKeyReleased(KeyEventArgs& e);

		/**
		* Invoked when the mouse is moved over the registered window
		*/
		virtual void OnMouseMoved(MouseMotionEventArgs& e);

		/**
		* Invoked when a mouse buttons is pressed over the registered window
		*/
		virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

		/**
		* Invoked when mouse button is released over the registered window
		*/
		virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

		/**
		* Invoked when the mouse wheel is scrolled while the registered window has focus
		*/
		virtual void OnMouseWheel(MouseWheelEventArgs& e);

		/**
		* Invoked when the attached window is resized
		*/
		virtual void OnResize(ResizeEventArgs& e);

		/**
		* Invoked when the registered window instance is destroyed
		*/
		virtual void OnWindowDestroy();

		std::shared_ptr<Window> m_pWindow;
	private:
		std::wstring m_Name;
		int m_Width;
		int m_Height;
		bool m_vSync;
		
	};							     
}								   

#endif // !GameIncluded
