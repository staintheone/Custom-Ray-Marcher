// Global variables 
cbuffer MatrixBuffer : register(b0) {
    matrix invView;
    float time;
    float3 padding;
};
cbuffer CameraBuffer : register(b1)
{
	float3 cameraPos;
	float camPadding;
}
cbuffer JuliaSeedBuffer : register(b2) {
    float3 juliaC; 
    float juliaPadding;
}

struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
// Shader entry point
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	float2 offset = cameraPos.xy;
	float zoom = pow(2.0, -cameraPos.z);
    // Map UV to a Plane
	float aspectRatio = 16.0 / 9.0;
	float2 z = input.uv * 2.0 - 1.0;
	z.x *= aspectRatio;
    
	z = z * zoom + offset;
    
    // Use ImGui C++ slider values for the seed
    float2 c = juliaC.xy; 

    //  The Iteration Loop
    int iterations = 0;
    const int maxIterations = 512;
    [loop] // Hint to compiler for performance
    for (int i = 0; i < maxIterations; i++)
    {
        // Complex multiplication: (x + yi)^2 = (x^2 - y^2) + (2xy)i
        float x2 = z.x * z.x;
        float y2 = z.y * z.y;
        
        if (x2 + y2 > 4.0) break; // Bailout
        
        z.y = 2.0 * z.x * z.y + c.y;
        z.x = x2 - y2 + c.x;
        
        iterations++;
    }

    // Coloring Logic
    if (iterations == maxIterations) return float4(0, 0, 0, 1); // Inside the set is black

    // Smooth coloring (prevents rings around fractal)
    float smoothIter = iterations + 1.0 - log2(log(length(z)));
    
    // Create apalette based on escaping speed
    float3 color = 0.5 + 0.5 * cos(3.0 + smoothIter * 0.15 + float3(0.0, 0.6, 1.0));
    
    return float4(color, 1.0);
}