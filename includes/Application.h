#pragma once 

// Includes
#include "../includes/Camera.h"
#include "../includes/Input.h"
#include "../includes/Rendering.h"
#include "../includes/Model.h"
#include "../includes/D3DCore.h"
#include "../includes/LightClass.h"
#include "../includes/imgui.h"
#include "../includes/imgui_impl_win32.h"
#include "../includes/imgui_impl_dx11.h"
#include "../includes/Timer.h"
#include "../includes/FPS.h"

// Globals
const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = false;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;	

class ApplicationClass
{
public:
	int input_array[8] = {0, 0, 0, 0, 0, 0, 0 , 0};
	float g_cameraRotation[3] = { 6.8f, 47.6f, 0.0f };
	float g_cameraPos[3] = { -2.735, 0.593f, -2.300f };
	bool drawUI = true;
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

	D3DCore* GetD3DCore();

private:
	bool Render();
	void RenderUI();

private:
	D3DCore* m_Direct3D;
	Camera* m_Camera;
	InputClass* m_Input;
	Model* m_Model;
	Rendering* m_Rendering;
	LightClass* m_Light;
	Timer* m_Timer;
	FPS* m_Fps;
	HWND m_hwnd;
	// IMGUI
	float frameTime;
	int g_fps;
	float g_frameTime;
	float m_frameTimeHistory[100] = { 0 };
	int m_historyIndex = 0;
	// Julia shader Parameters
	float juliaSeed[3] = { -0.7f, 0.45f, -0.1f };
	float animationSpeed = 5.0f;
	//////////////////////////
	//Mandelbulb shader parameters
	float Power = 8.0f;
	float fractalLOD = 30;
	float bailout = 2.0f;
	float epsilonVal = 0.001f;
	float surfThreshold = 0.001f;
	float maxIterations = 256;
	float stepMultiplier = 0.75f;
	float glowComponent = 0.05f;
	//////////////////////////////
	float colorA[4] = { 1.0, 0.7, 0.4, 1.0 };
	float colorB[4] = { 0.00, 0.15, 0.2, 1.0 };
};
