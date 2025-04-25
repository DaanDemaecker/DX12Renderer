#ifndef _COMMAND_LIST_
#define _COMMAND_LIST_

// File includes
#include "Includes/DirectXIncludes.h"

namespace DDM
{
	class CommandList final
	{
	public:
		// Default constructor
		CommandList();
		~CommandList();

		// Delete copy and move operations
		CommandList(CommandList& other) = delete;
		CommandList(CommandList&& other) = delete;

		CommandList& operator=(CommandList& other) = delete;
		CommandList& operator=(CommandList&& other) = delete;


	private:

	};
}


#endif // _COMMAND_LIST_