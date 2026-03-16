// Pixel Shader.

// Global variables
cbuffer MatrixBuffer : register(b0)
{
    matrix invView;
    float time;
    float3 padding;
};
cbuffer CameraBuffer : register(b1)
{
    float3 cameraPos;
    float camPadding;
};
cbuffer fractalColor : register(b4)
{
    float4 colorA;
    float4 colorB;
};
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
// A helper func to create smooth near-random colors
float3 GetColor(float t)
{
    // cosine palette: (Adjust values to change color theme)
    float3 a = float3(0.2, 0.3, 0.5);
    float3 b = float3(0.5, 0.8, 0.5);
    float3 c = colorA.rgb;
    float3 d = colorB.rgb;
    
	return a + b * cos(6.28318 * (c * t + d));
} 
// PixelShader Begin
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
    // Setup Screen Coordinates
	float aspectRatio = 16.0 / 9.0;
	float2 uv = input.uv * 2.0 - 1.0;
	uv.x *= aspectRatio;
	
    // Navigation
	float zoom = pow(0.5, cameraPos.z);
    // Map UVs to the Complex Plane (c = x + yi)
    float2 c = uv * zoom + cameraPos.xy;
	
    // The Mandelbrot Set Logic (z = z^2 + c)
	float2 z = float2(0.0, 0.0);
	float iter = 0.0;
	const float maxIter = 128.0; // increase for more details

	for (float i = 0; i < maxIter; i++)
	{
        // Optimization: Pre-calculate squares
		float x2 = z.x * z.x;
		float y2 = z.y * z.y;

        // Escape condition: |z| > 2
		if (x2 + y2 > 4.0)
			break;

        // Complex number: z = z^2 + c
        // Real: x^2 - y^2 + cx
        // Imaginary: 2xy + cy
		z.y = 2.0 * z.x * z.y + c.y;
		z.x = x2 - y2 + c.x;
        
		iter++;
	}

    // Smooth Coloring Logic
    // This removes the staircase effect between iteration levels
	if (iter < maxIter)
	{
        // Fractional iteration formula
		float log_zn = log(z.x * z.x + z.y * z.y) / 2.0;
		float nu = log(log_zn / log(2.0)) / log(2.0);
		iter = iter + 1.0 - nu;
	}
    // Final Shading
	float3 finalColor;
    
	if (iter >= maxIter)
	{
        // Interior of the Mandelbrot set is usually black
		finalColor = float3(0.0, 0.0, 0.0);
	}
	else
	{
        // Use the smoothing iteration count to sample the palettes
		float t = iter / 128.0; // Scale by a factor to control color density
		finalColor = GetColor(t + time * 0.1); // Added time for color cycling
	}

	return float4(finalColor, 1.0);
}