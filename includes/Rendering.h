#pragma once

// Includes
#include<d3d11.h>
#include<d3dcompiler.h>
#include<directxmath.h>
#include<fstream>

// namespaces
using namespace DirectX;
using namespace std;

class Rendering
{
private:

	struct MatrixBufferType
	{
		XMMATRIX invView;   // The inverse view matrix for ray direction
		float time;         // Total elapsed time for animation
		XMFLOAT3 padding;   // Packing to hit 16-byte alignment
	};
	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};
	struct juliaSeedBufferType
	{
		XMFLOAT3 juliaSeed;
		float padding;
	};
	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float specularPower;
		XMFLOAT4 specularColor;
	};
	struct mandelbulbSettingsBufferType
	{
		float bufferFractalPower;
		float bufferFractalLOD;
		float bufferBailout;
		float bufferEpsilonVal;
		float bufferSurfThreshold;
		float bufferMaxIterations;
		float bufferStepMultiplier;
		float bufferGlowComponent;
	};
	struct fractalColorBufferType
	{
		XMFLOAT4 colorA;
		XMFLOAT4 colorB;
	};
public:
	Rendering();
	Rendering(const Rendering&);
	~Rendering();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, XMMATRIX, XMFLOAT3, float, float[3], float, float, float, float, float, float, float, float, float, XMFLOAT4, XMFLOAT4);
	bool ReloadShaders(ID3D11Device*, HWND);
	bool ChangeShader(ID3D11Device* device, HWND hwnd, const wchar_t* filename);
private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMFLOAT3, float, float[3], float, float, float, float, float, float, float, float, float, XMFLOAT4, XMFLOAT4);
	void RenderShader(ID3D11DeviceContext*);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_juliaSeedBuffer;
	ID3D11Buffer* m_mandelBulbSettingsBuffer;
	ID3D11Buffer* m_fractalColorBuffer;
};