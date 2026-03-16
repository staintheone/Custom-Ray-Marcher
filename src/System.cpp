#include "../includes/System.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define FULL_SCREEN true
SystemClass::SystemClass()
{
	m_hinstance = 0;
	m_input = 0;
	m_Application = 0;
	m_Timer = 0;
	m_applicationName = 0;
	m_camera = 0;
	m_hwnd = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{

}

SystemClass::~SystemClass()
{

}

bool SystemClass::Initialize()
{
	RAWINPUTDEVICE rid;
	int screenWidth, screenHeight;
	bool result;

	// INit the widht and height of the screen to zero before sending the variables to the function.
	screenHeight = 0;
	screenWidth = 0;

	// Init the windows api
	InitializeWindows(screenWidth, screenHeight);

	// create and init the input object. this object will be used to handel reading keyboard inputs from user.
	m_input = new InputClass;

	m_Timer = new Timer();
	m_Timer->Initialize();

	m_input -> Initialize();
	rid.usUsagePage = 0x01;          // Generic Desktop Controls
	rid.usUsage = 0x02;              // Mouse
	rid.dwFlags = 0;                 // Use 0 for default (no capture outside window)
	rid.hwndTarget = m_hwnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	// Create and init the application class object. this obj will handle rendering all the graphics for that application.
	m_Application = new ApplicationClass;

	result = m_Application->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	return true;
}

void SystemClass::Shutdown()
{
	// Release Timer object
	if (m_Timer) {
		delete m_Timer;
		m_Timer = 0;
	}
	// Release all the application class object
	if (m_Application)
	{
		m_Application->Shutdown();
		delete m_Application;
		m_Application = 0;
	}

	// Release the input object
	if (m_input)
	{
		delete m_input;
		m_input = 0;
	}

	// Shutdown the window;
	ShutdownWindows();

	return;
}

void SystemClass::Run() {
	MSG msg;
	bool done, result;

	// Init the message structure.

	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
		// Handle the window message
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Continue the frame processing 
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}
	}
	return;
}

bool SystemClass::Frame()
{
	bool result;
	m_Timer->Frame();
	float dt = m_Timer->GetTime();
	float speed =  dt * 0.7f;
	float zoom = pow(0.5f, m_Application->g_cameraPos[2]);
	float adaptiveSpeed = speed * zoom;

	float yaw = m_Application->g_cameraRotation[1] * 0.0174532925f;

	// Check if the user has pressed escape key and wants to exit
	if (m_input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}		
	if (m_input->IsKeyDown(VK_F1))
	{
		m_Application->drawUI = false;
	}
	if (m_input->IsKeyDown(VK_F2))
	{
		m_Application->drawUI = true;
	}
	// input detection for Camera movement 
	if (m_input->IsKeyDown('W')) {
		m_Application->g_cameraPos[0] += sinf(yaw) * adaptiveSpeed;
		m_Application->g_cameraPos[2] += cosf(yaw) * adaptiveSpeed;
	}
	if (m_input->IsKeyDown('S')) {
		m_Application->g_cameraPos[0] -= sinf(yaw) * adaptiveSpeed;
		m_Application->g_cameraPos[2] -= cosf(yaw) * adaptiveSpeed;
	}
	if (m_input->IsKeyDown('D')) {
		m_Application->g_cameraPos[0] += cosf(yaw) * adaptiveSpeed;
		m_Application->g_cameraPos[2] -= sinf(yaw) * adaptiveSpeed;
	}
	if (m_input->IsKeyDown('A')) {
		m_Application->g_cameraPos[0] -= cosf(yaw) * adaptiveSpeed;
		m_Application->g_cameraPos[2] += sinf(yaw) * adaptiveSpeed;
	}
	if (m_input->IsKeyDown('E'))
	{
		m_Application->g_cameraPos[1] += adaptiveSpeed;
	}
	if (m_input->IsKeyDown('Q'))
	{
		m_Application->g_cameraPos[1] -= adaptiveSpeed;
	}
	
	// Do the Frame processing for the application class object.
	result = m_Application->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_INPUT:
		{
			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpb[sizeof(RAWINPUT)];

			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
			RAWINPUT* raw = (RAWINPUT*)lpb;

			if(m_input->IsKeyDown(VK_RBUTTON))
			{
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					// mouse.lLastX and lLastY are the relative movement deltas
					int deltaX = raw->data.mouse.lLastX;
					int deltaY = raw->data.mouse.lLastY;

					// Update the rotation matrices of camera accordingly of mouse deltas
					m_Application->g_cameraRotation[1] += deltaX * 0.1f;
					m_Application->g_cameraRotation[0] -= deltaY * 0.1f;
				
					if (m_Application->g_cameraRotation[0] > 89.0f) m_Application->g_cameraRotation[0] = 89.0f;
					if (m_Application->g_cameraRotation[0] < -89.0f) m_Application->g_cameraRotation[0] = -89.0f;
				}
			}
			return 0;
		}
		// Change rotation only if the right mouse button is held.
		case WM_RBUTTONDOWN:
			m_input->KeyDown(VK_RBUTTON);
			return 0;
		case WM_RBUTTONUP:
			m_input->KeyUp(VK_RBUTTON);
			return 0;

		// Check if a key has been pressed or not on the keyboard.
		case WM_KEYDOWN:
		{
			// If a key is pressed send it to the input object so it can record that state
			m_input->KeyDown((unsigned int)wparam);
			return 0;
		}

		// Check if the key has been released on the keyboard
		case WM_KEYUP:
		{
			// If a key  has been released then send it to input object...
			m_input->KeyUp((unsigned int)wparam);
			return 0;
		}

		// Any other message send to default message handler as out application will not use the,
		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
}
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get an external pointer to this object.
	ApplicationHandle = this;

	// Get the instance of this application handle
	m_hinstance = GetModuleHandle(NULL);

	// Give application a name
	m_applicationName = L"Ray Marcher - Remora";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the res of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode
	if (FULL_SCREEN)
	{
		// if full screen set the screen to maximum size of the users desktop.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display setting to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
							WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
							posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(true);

	return;
}

void SystemClass::ShutdownWindows()
{
	// Show mouse cursor in window.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);	
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam))
	{
		return true;
	}
	switch (umessage)
	{
		// Check if the window is being destroyed
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE:
		{
			if (wparam != SIZE_MINIMIZED)
			{
				int newWidth = LOWORD(lparam);
				int newHeight = HIWORD(lparam);

				if (ApplicationHandle && ApplicationHandle->GetD3DCore())
				{
					ApplicationHandle->GetD3DCore()->OnResize(newWidth, newHeight);
				}
			}
			return 0;
		}
		// All other messages are sent as a default message.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
D3DCore* SystemClass::GetD3DCore()
{
	return m_Application ? m_Application->GetD3DCore() : nullptr;
}