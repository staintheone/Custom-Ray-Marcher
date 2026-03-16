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
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// Simple Sphere Distance Estimator
float SphereDE(float3 p)
{
	float3 center = float3(0, 0, 0);
	float pulse = sin(time * 2.0) * 0.25;
	float radius = 1.0 + pulse ;
    
    // Make the sphere "pulse" using the time variable
	radius += sin(time * 2.0) * 0.5;
    
	return length(p - center) - radius; // returns a sphere SDF
}

//Calculate the surface normal using a gradient
float3 GetNormal(float3 p) {
    float2 e = float2(0.001, 0.0);
    float d = SphereDE(p);
    float3 n = d - float3(
        SphereDE(p - e.xyy),
        SphereDE(p - e.yxy),
        SphereDE(p - e.yyx)
    );
    return normalize(n);
}

// PixelShader Begin
float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	//Convert UV to Screen Space (-1 to 1)
    float aspectRatio = 16.0 / 9.0; // manually take the aspect ratio in account 
    float2 p = input.uv * 2.0 - 1.0;
    p.x *= aspectRatio;
	
	//Setup the Ray
    // Use invView to transform the ray direction based on camera rotation
    float3 rayDir = normalize(mul((float3x3)invView, float3(p, 1.5))); 
    float3 rayOrigin = cameraPos;
	
	//Ray Marching Loop
    float t = 0.0;      // Total distance traveled
    for(int i = 0; i < 64; i++) 
    {
        float3 currPos = rayOrigin + rayDir * t;
        float d = SphereDE(currPos);
        
        if(d < 0.001) {
            // --- PHONG LIGHTING CALCULATIONS ---
            
            float3 N = GetNormal(currPos);               // Surface Normal
            float3 L = normalize(float3(5.0, 5.0, -5.0)); // Light Position (World)
            float3 V = -rayDir;                          // View Direction (towards camera)
            float3 R = reflect(-L, N);                   // Reflected Light vector

            // Ambient: Basic constant light
            float3 ambient = float3(0.1, 0.1, 0.2); 

            // Diffuse: Standard matte shading
            float dotNL = max(dot(N, L), 0.0);
            float3 diffuse = float3(0.8, 0.5, 0.2) * dotNL;

            // Specular: The bright spot highlight
            float specularStrength = 0.5;
            float shininess = 32.0;
            float dotRV = max(dot(R, V), 0.0);
            float3 specular = float3(1.0, 1.0, 1.0) * pow(dotRV, shininess) * specularStrength;

            // Final Color
            return float4(ambient + diffuse + specular, 1.0);
        }
        
        if(t > 20.0) break; // Missed everything
        t += d;
    }

    // Background (Sky/Space)
    return float4(0.05, 0.05, 0.1, 1.0);
    //return float4(rayDir * 0.5 + 0.5, 1.0);
}