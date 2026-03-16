#include "../includes/Rendering.h"

Rendering::Rendering()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_cameraBuffer = 0;
	m_juliaSeedBuffer = 0;
	m_fractalColorBuffer = 0;
	m_mandelBulbSettingsBuffer = 0;
}

Rendering::Rendering(const Rendering&)
{

}

Rendering::~Rendering()
{

}

bool Rendering::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;

	// Set the file name for vertex shader C:\\MyProject\\resources\\shader.vs C:\\Remora Engine\\resources\\shader.vs
	error = wcscpy_s(vsFilename, 128, L"resources/vertexShader.hlsl");
	if (error != 0)
	{
		return false;
	}

	// Set the filename for pixel shader.
	error = wcscpy_s(psFilename, 128, L"resources/sphere.hlsl");
	if (error != 0)
	{
		return false;
	}

	// Init vertex and pixel shader files.
	result = InitializeShader(device, hwnd, vsFilename, psFilename);
	if (!result)
	{
		return false;
	}
	return true;
}

void Rendering::Shutdown()
{
	ShutdownShader();

	return;
}

bool Rendering::Render(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMFLOAT3 cameraPos, float totalTime, float juliaSeedVal[3], float animationSpeed,
						float Power, float fractalLOD, float bailout, float epsilonVal, float surfThreshold, float maxIterations, float stepMultiplier, float glowComponent,
						XMFLOAT4 colorA, XMFLOAT4 colorB)
{
	bool result;

	// Set the shader parameters that will be used for rendering.

	result = SetShaderParameters(deviceContext, viewMatrix, cameraPos, totalTime, juliaSeedVal, animationSpeed , Power,  fractalLOD,  bailout,  epsilonVal,  surfThreshold,  maxIterations,  stepMultiplier,  glowComponent, colorA, colorB);
	if (!result)
	{
		return false;
	}

	// Now the render
	RenderShader(deviceContext);

	return true;
}
bool Rendering::ReloadShaders(ID3D11Device* device, HWND hwnd)
{
	ShutdownShader();

	return Initialize(device, hwnd);
}
bool Rendering::ChangeShader(ID3D11Device* device, HWND hwnd, const wchar_t* filename)
{
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;

	ShutdownShader(); // Clear the old shader

	error = wcscpy_s(vsFilename, 128, L"resources/vertexShader.hlsl");
	if (error != 0)
	{
		return false;
	}

	// Set the filename for pixel shader.
	error = wcscpy_s(psFilename, 128, filename);
	if (error != 0)
	{
		return false;
	}

	return InitializeShader(device, hwnd, vsFilename, psFilename);
}
bool Rendering::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC juliaSeedBufferDesc;
	D3D11_BUFFER_DESC mandelbulbBufferDesc;
	D3D11_BUFFER_DESC fractalColorBufferDesc;

	// Init the pointer this function will use, setting it to null
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the vertex shader code.

	result = D3DCompileFromFile(vsFilename, NULL, NULL, "VS_Main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader compilation fails, it will write an error message.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Vertex Shader file", MB_OK);
		}
		return false;
	}

	// Compile the pixel shader file 
	result = D3DCompileFromFile(psFilename, NULL, NULL, "PS_Main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		 &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// Write the error message if shader compilation fails.
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Pixel Shader File!", MB_OK);
		}
		return false;
	}

	// Create vertex shader from the buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	// Create pixel shader from the buffer 
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since we don't need it anymore
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Set up the dynamic matrix const buffer that is present in the vertex shader
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the const buffer pointer so we can access the vertex shader const buffer from within the class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType); // 16 bytes
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the const buffer pointer to camera buffer so we can access the vertex shader const buffer from within the class.
	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result)) 
	{
		return false;
	}

	juliaSeedBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	juliaSeedBufferDesc.ByteWidth = sizeof(juliaSeedBufferType);
	juliaSeedBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	juliaSeedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	juliaSeedBufferDesc.MiscFlags = 0;
	juliaSeedBufferDesc.StructureByteStride = 0;

	// Create the Julia Fractal seed constant buffer so we can pass the seed runtime and change the shape 
	result = device->CreateBuffer(&juliaSeedBufferDesc, NULL, &m_juliaSeedBuffer);
	if (FAILED(result))
	{
		return false;
	}
	
	mandelbulbBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mandelbulbBufferDesc.ByteWidth = sizeof(mandelbulbSettingsBufferType);
	mandelbulbBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mandelbulbBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mandelbulbBufferDesc.MiscFlags = 0;
	mandelbulbBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&mandelbulbBufferDesc, NULL, &m_mandelBulbSettingsBuffer);
	if (FAILED(result))
	{
		return false;
	}

	fractalColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	fractalColorBufferDesc.ByteWidth = sizeof(fractalColorBufferType);
	fractalColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	fractalColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	fractalColorBufferDesc.MiscFlags = 0;
	fractalColorBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&fractalColorBufferDesc, NULL, &m_fractalColorBuffer);
	if (FAILED(result))
	{
		return false;
	}
	
	return true;
}

void Rendering::ShutdownShader()
{
	// Release the camera buffer
	if (m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = 0;
	}
	// Release the matrix const buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}
	if (m_juliaSeedBuffer)
	{
		m_juliaSeedBuffer->Release();
		m_juliaSeedBuffer = 0;
	}
	// Release the layout
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}
	// Release the pixel shader
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}
	// Release the vertex shader.
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
	return;
}
void Rendering::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get a length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write the buffer to this file.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// pop a message saying the shader compilation failed and prompt the user to check the log file.
	MessageBox(hwnd, L"Error Compiling shader ! check shader-error.txt for error description", shaderFilename, MB_OK);

	return;
}

bool Rendering::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMFLOAT3 cameraPosition, float totalTime, float juliaSeedVal[3], float animationSpeed,
	float Power, float fractalLOD, float bailout, float epsilonVal, float surfThreshold, float maxIterations, float stepMultiplier, float glowComponent, XMFLOAT4 colorA, XMFLOAT4 colorB)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResources;
	MatrixBufferType* dataPtr; 
	CameraBufferType* camPtr;
	juliaSeedBufferType* juliaPtr;
	mandelbulbSettingsBufferType* mandelbulbPtr;
	fractalColorBufferType* fractalColorPtr;

	//Calculate Inverse View Matrix
	XMMATRIX invView = XMMatrixInverse(NULL, viewMatrix);
	//invView = XMMatrixTranspose(invView); // HLSL transpose

	// Lock the const buffer so it can be written into.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the const buffer
	dataPtr = (MatrixBufferType*)mappedResources.pData;

	// Copy the matrices into the const buffer
	dataPtr->invView = invView;
	dataPtr->time = totalTime;

	// Unlock the const buffer
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// Lock the camera constant buffer so it can be written to.
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	camPtr = (CameraBufferType*)mappedResources.pData;

	// Copy the camera position into the constant buffer.
	camPtr->cameraPosition = cameraPosition;
	camPtr->padding = 0.0f;

	// Unlock the camera constant buffer.
	deviceContext->Unmap(m_cameraBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &m_cameraBuffer);

	// Lock the Julia seed buffer so it can be written
	result = deviceContext->Map(m_juliaSeedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer
	juliaPtr = (juliaSeedBufferType*)mappedResources.pData;

	// Copy the julia seed into a constant buffer
	juliaPtr->juliaSeed.x = juliaSeedVal[0];
	juliaPtr->juliaSeed.y = juliaSeedVal[1];
	juliaPtr->juliaSeed.z = juliaSeedVal[2];
	juliaPtr->padding = 0.0;

	// Unlock the julia constant buffer.
	deviceContext->Unmap(m_juliaSeedBuffer, 0);
	deviceContext->PSSetConstantBuffers(2, 1, &m_juliaSeedBuffer);

	// Lock the mandelbulb settings buffer so we can write into it.
	result = deviceContext->Map(m_mandelBulbSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	if (FAILED(result))
	{
		return false;
	}

	// Get the pointer to the data in buffer
	mandelbulbPtr = (mandelbulbSettingsBufferType*)mappedResources.pData;

	// copy the values of settings into a const buffer
	mandelbulbPtr->bufferFractalPower = Power;
	mandelbulbPtr->bufferFractalLOD = fractalLOD;
	mandelbulbPtr->bufferBailout = bailout;
	mandelbulbPtr->bufferEpsilonVal = epsilonVal;
	mandelbulbPtr->bufferSurfThreshold = surfThreshold;
	mandelbulbPtr->bufferMaxIterations = maxIterations;
	mandelbulbPtr->bufferStepMultiplier = stepMultiplier;
	mandelbulbPtr->bufferGlowComponent = glowComponent;

	// Unlock the julia constant buffer.
	deviceContext->Unmap(m_mandelBulbSettingsBuffer, 0);
	deviceContext->PSSetConstantBuffers(3, 1, &m_mandelBulbSettingsBuffer);

	// Lock the fractal color settings buffer so we can write into it.
	result = deviceContext->Map(m_fractalColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
	if (FAILED(result))
	{
		return false;
	}

	fractalColorPtr = (fractalColorBufferType*)mappedResources.pData;

	fractalColorPtr->colorA = colorA;
	fractalColorPtr->colorB = colorB;

	// Unlock the fractal color constant buffer.
	deviceContext->Unmap(m_fractalColorBuffer, 0);
	deviceContext->PSSetConstantBuffers(4, 1, &m_fractalColorBuffer);

	return true;
}

void Rendering::RenderShader(ID3D11DeviceContext* deviceContext)
{
	// Set the vertex and pixel shader that will be used for rendering.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Standard triangle list topology
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// render the scene.
	deviceContext->Draw(3, 0);

	return;
}

