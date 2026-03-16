#include "../includes/System.h"

// WinMain is the entry point for any windows (win32) application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass* System;
	bool result;

	// Create the system obj.
	System = new SystemClass();

	// Init and run the system object.
	result = System->Initialize();
	if (result)
	{
		System->Run();
	}
	// Shutdown and release the system object when window is destroyed.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}