#pragma once // Header gaurd

// Pre-processors
#define WIN32_LEAN_AND_MEAN

// System headers

#include<Windows.h>

// User headers.
#include"Application.h"
#include "Camera.h"
#include "../includes/Timer.h"
#include"Input.h"
#include "../includes/imgui_impl_dx11.h"
#include "../includes/imgui.h"
#include "../includes/imgui_impl_win32.h"

// SystemClass Code begins.

class SystemClass
{
public:
	float pos[3] = {};
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	D3DCore* GetD3DCore();

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_input;
	Timer* m_Timer;
	Camera* m_camera;
	ApplicationClass* m_Application;


};

// Function Prototypes(Helper funcitons for windows itslef)
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global handles
static SystemClass* ApplicationHandle = 0;