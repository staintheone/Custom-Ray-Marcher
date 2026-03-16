#include "../includes/Application.h"

ApplicationClass::ApplicationClass()
{
	// Set values to 0 at object creation
	m_Direct3D = 0;
	m_Camera = 0;
	m_Rendering = 0;
	m_Input = 0;
	m_Fps = 0;

	g_cameraPos[2] = -3.0f; // Default position of camera (negetive on z axis).
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	// Create and init the Direct3D object in D3DCore file.
	m_Direct3D = new D3DCore;

	// Initialize Direct3D(DirectX11).
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not init DirectX 11", L"error", MB_OK);
	}

	// Create timer object
	m_Timer = new Timer;
	if (!m_Timer)
	{
		return false;
	}
	// Create FPS object
	m_Fps = new FPS;
	if (!m_Fps)
	{
		return false;
	}

	// Initialize the FPS and Timerobject.
	m_Fps->Initialize();
	m_Timer->Initialize();

	// Initialize imGUI for quick and easy to use UI.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(
		m_Direct3D->GetDevice(),
		m_Direct3D->GetDeviceContext()
	);
	
	// Create camera object
	m_Camera = new Camera;

	// Set init position of camera
	m_Camera->SetPosition(-5.021f, 1.167f, -4.289f);

	// Create rendering object for rendering class
	m_Rendering = new Rendering;

	// Initialize rendering 
	result = m_Rendering->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Rendering code", L"error", MB_OK);
		return false;
	}

	return true;
}

void ApplicationClass::Shutdown()
{
	// Release the FPS class obj
	if (m_Fps) {
		delete m_Fps;
		m_Fps = 0;
	}
	// Release the Light class OBJ
	if (m_Timer) {
		delete m_Timer;
		m_Timer = 0;
	}
	
	// Release the Rendering obj
	if (m_Rendering)
	{
		m_Rendering->Shutdown();
		delete m_Rendering;
		m_Rendering = 0;
	}

	// Release the camera obj
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Shutdown imGUI.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Release the DirectX	
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}

bool ApplicationClass::Frame()
{
	bool result;

	// Get frame count.
	m_Fps->Frame();
	m_Timer->Frame();

	g_fps = m_Fps->GetFps(); // copy the fps to global variable for imGUI.
	g_frameTime = m_Timer->GetTime() * 1000.0f; // Calculate frame time.

	// Render the next frame.
	result = Render();
	if (!result)
	{
		return false;
	}
	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX viewMatrix;
	bool result;
	static float totalTime = 0.0f;
	totalTime += (m_Timer->GetTime() / 1000.0f);
	float range = 0.95f;
	float baseCX = -1.2f;
	float baseCY = -1.2f;

	if (g_cameraPos[2] < -5.01f)
	{
		g_cameraPos[2] = -5.0f;
	}
	if (g_cameraPos[0] < -5.01f)
	{
		g_cameraPos[0] = -5.0f;
	}
	// Clear the buffer and begin the scene
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	m_Camera->SetPosition(g_cameraPos[0], g_cameraPos[1], g_cameraPos[2]); // set updated camera position
	m_Camera->SetRotation(g_cameraRotation[0], g_cameraRotation[1], g_cameraRotation[2]); // set camera rotation updated by imGUI.

	m_Camera->Render();
	// Get the world, view and projection matrices from the camera and d3d objects
	m_Camera->GetViewMatrix(viewMatrix);

	juliaSeed[0] = baseCX + (sin(totalTime * animationSpeed) * range);
	juliaSeed[1] = baseCY + (cos(totalTime * (animationSpeed * 0.8f)) * range);
	
	// Render the model using Rendering class
	result = m_Rendering->Render(m_Direct3D->GetDeviceContext(), viewMatrix, m_Camera->GetPosition(), totalTime, juliaSeed, animationSpeed, 
								Power, fractalLOD, bailout, epsilonVal, surfThreshold, maxIterations, stepMultiplier, glowComponent, 
								XMFLOAT4(colorA[0], colorA[1], colorA[2], colorA[3]), XMFLOAT4(colorB[0], colorB[1], colorB[2], colorB[3]));
	if (!result)
	{
		return false;
	}
	if (drawUI) {
		RenderUI();
	}
	// Present the rendered scene to screen.
	m_Direct3D->EndScene();

	return true;
}
void ApplicationClass::RenderUI()
{
	static float totalTime = 0.0f;
	totalTime += (m_Timer->GetTime() / 1000.0f);
	float range = 0.95f;
	float baseCX = -1.2f;
	float baseCY = -1.2f;

	// Initialize next imGUI frame.
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// IMGui window for changing the shader file
	//***************************//
	//////////////////////////////////

	// imGUI window for scene statistics, fps and frametime.
	ImGui::Begin("Scene Stats");
	ImGui::PushItemWidth(700.0f);

	ImGui::Text("FPS : %d ", g_fps);
	ImGui::Text("Frame Time : %.4f ms ", g_frameTime);

	m_frameTimeHistory[m_historyIndex] = g_frameTime; // build an array of frametime that will be plotted in graph
	m_historyIndex = (m_historyIndex + 1) % 100;

	ImGui::PlotLines(
		"##FrameGraph",          // Label
		m_frameTimeHistory,      // The data
		100,                     // Number of points
		m_historyIndex,          // The "start" point (helps the graph scroll)
		"Frame Time (ms)",       // Text overlay in the middle
		0.0f,                    // Scale Min
		33.0f,                   // Scale Max 
		ImVec2(0, 80)            // Graph size (0 = width, 80 = height)
	);

	ImGui::End();
	///////////////////////////////////

	// imGUI window for camera controls info menu 
	ImGui::Begin("Controls Info");

	ImGui::Text("Move forward : W ");
	ImGui::Text("Move Backwards : S ");
	ImGui::Text("Move Left : A ");
	ImGui::Text("Move Right : D");
	ImGui::Text("Move Downwards : Q ");
	ImGui::Text("Move Upwards : E ");
	ImGui::Text("Hold down Right mouse button to look around");
	ImGui::Text("F1 to hide the UI, F2 to show the UI again");

	ImGui::End();
	//////////////////////////////////

	// imGUI window for camera settings.
	ImGui::Begin("Camera Settings");
	ImGui::PushItemWidth(500.0f);

	ImGui::SliderFloat3("Camera Position", g_cameraPos, -5.0f, 20.0f);
	ImGui::SliderFloat3("Camera Rotation", g_cameraRotation, -20.0f, 20.0f);
	ImGui::End();
	///////////////////////////////////

	// IMGui button for hot-reloading shaders
	if (ImGui::Button("Reload Shaders"))
	{
		bool result = m_Rendering->ReloadShaders(m_Direct3D->GetDevice(), m_hwnd);
	}
	///////////////////////////////////

	ImGui::Begin("Shader Select");

	if (ImGui::Button("Load Sphere")) {
		m_Rendering->ChangeShader(m_Direct3D->GetDevice(), m_hwnd, L"resources/sphere.hlsl");
		g_cameraPos[0] = -2.735;
		g_cameraPos[1] = 0.593;
		g_cameraPos[2] = -2.300;
	}

	if (ImGui::Button("Load Mandelbulb")) {
		m_Rendering->ChangeShader(m_Direct3D->GetDevice(), m_hwnd, L"resources/mandelbulb.hlsl");
		g_cameraPos[0] = -2.735;
		g_cameraPos[1] = 0.593;
		g_cameraPos[2] = -2.300;
		//colorA[1] = XMFLOAT4(0.23, 0.30, 0.22, 1.0);
		colorA[0] = 0.23;
		colorA[1] = 0.30;
		colorA[2] = 0.22;
		colorB[0] = 1.0;
		colorB[1] = 0.80;
		colorB[2] = 0.4;
	}

	if (ImGui::Button("Load 2D mandelbrot set")) {
		totalTime = 0;
		g_cameraPos[0] = -1.311;
		g_cameraPos[1] = 0.019;
		g_cameraPos[2] = 1.424;
		colorA[0] = 1.0;
		colorA[1] = 0.70;
		colorA[2] = 0.40;
		colorB[0] = 0.0;
		colorB[1] = 0.15;
		colorB[2] = 0.2;
		m_Rendering->ChangeShader(m_Direct3D->GetDevice(), m_hwnd, L"resources/2Dmandelbrot.hlsl");
	}
	if (ImGui::Button("Load The Julia set")) {
		totalTime = 0;
		m_Rendering->ChangeShader(m_Direct3D->GetDevice(), m_hwnd, L"resources/julia.hlsl");
		g_cameraPos[0] = -2.735;
		g_cameraPos[1] = 0.593;
		g_cameraPos[2] = -2.300;
	}
	if (ImGui::Button("load 2D Julia Set"))
	{
		totalTime = 0;
		m_Rendering->ChangeShader(m_Direct3D->GetDevice(), m_hwnd, L"resources/2Djulia.hlsl");
		g_cameraPos[0] = -0.085;
		g_cameraPos[1] = 0.036;
		g_cameraPos[2] = -0.161;
	}
	ImGui::End();

	//Julia shader control panel
	ImGui::Begin("Julia Shader Control");
	ImGui::SliderFloat3("Change Seed", juliaSeed, -10.0f, 10.0f);
	ImGui::SliderFloat("Change animation speed", &animationSpeed, 0.0f, 30.0f);
	ImGui::End();
	//////////////////////////////

	// Mandelbulb Shader control panel
	ImGui::Begin("Mandelbulb settings");
	ImGui::SliderFloat("Power of the fractal", &Power, 0.0f, 20.0f);
	ImGui::SliderFloat("LOD of the fractal", &fractalLOD, 0.0f, 512.0f);
	ImGui::SliderFloat("Epsilon Value", &epsilonVal, 0.0001f, 0.05f, "%.4f");
	ImGui::SliderFloat("Surface threshold of the fractal ", &surfThreshold, 0.0001f, 0.1f, "%.4f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Bailout of the fractal", &bailout, 0.0f, 10.0f);
	ImGui::SliderFloat("Ray march maximum steps of the fractal", &maxIterations, 0.0f, 1024.0f);
	ImGui::SliderFloat("steps multipler of the fractal", &stepMultiplier, 0.0f, 1.0f);
	ImGui::SliderFloat("Glow component the fractal", &glowComponent, 0.0f, 1.0f);
	ImGui::End();
	//////////////////////////////

	// Color Slider for the fractals
	ImGui::Begin("Mandelbulb Color settings");
	ImGui::ColorEdit4("Primary Color", colorA);
	ImGui::ColorEdit4("Secondary Color", colorB);
	ImGui::End();
	//////////////////////////////

	ImGui::Render(); // Final imGUI draw call.
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return;
}
D3DCore* ApplicationClass::GetD3DCore()
{
	return m_Direct3D; // helper function for getting D3DCore reference pointer.
}
