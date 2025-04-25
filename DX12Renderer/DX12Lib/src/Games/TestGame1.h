// TestGame1.h
// This class will serve as a testing class for the Game class

#ifndef TestGame1Included
#define TestGame1Included

// File includes
#include "Games/Game.h"

namespace DDM
{
	class TestGame1 : public Game
	{
	public:
		TestGame1(const std::wstring& name, int width, int height, bool vSync);

		~TestGame1();

		/**
		* Load content required for the game.
		*/
		virtual bool LoadContent();

		/**
		* Unload game specific content that was loaded in LoadContent.
		*/
		virtual void UnloadContent();

	private:

	};
}


#endif // !TestGame1Included
