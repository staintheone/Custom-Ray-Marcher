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
cbuffer JuliaSeedBuffer : register(b2)
{
	float3 juliaC;
	float juliaPadding;
}
struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

static float orbitTrap = 0.0;
static const float Power = 8.0;

// This is the seed. Changing this changes the entire shape of fractal.
float Julia3DDE(float3 p)
{
	float3 z = p;
	float dr = 1.0;
	float r = 0.0;
	float oTrap = 1e10;

	for (int i = 0; i < 20; i++)
	{
		r = length(z);
		if (r > 2.0)
			break;

		dr = pow(r, Power - 1.0) * Power * dr + 1.0;

		float theta = acos(z.z / r);
		float phi = atan2(z.y, z.x);
        
		float zr = pow(r, Power);
		theta *= Power;
		phi *= Power;

		z = zr * float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        
        // --- JULIA DIFFERENCE ---
        // add the constant seed
		z += juliaC;
        
		//oTrap = min(oTrap, length(z) - 0.2);
		oTrap = min(oTrap, abs(z.y));
	}
    
	orbitTrap = oTrap;
	return 0.5 * log(r) * r / dr;
}

float3 GetNormal(float3 p, float t)
{
	float e = 0.001;
	float2 h = float2(1.0, -1.0) * 0.5773;
	return normalize(
        h.xyy * Julia3DDE(p + h.xyy * e) +
        h.yyx * Julia3DDE(p + h.yyx * e) +
        h.yxy * Julia3DDE(p + h.yxy * e) +
        h.xxx * Julia3DDE(p + h.xxx * e)
    );
}

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	float aspectRatio = 16.0 / 9.0;
	float2 uv = input.uv * 2.0 - 1.0;
	uv.x *= aspectRatio;
    
	float3 rayDir = normalize(mul((float3x3) invView, float3(uv, 1.5)));
	float3 rayOrigin = cameraPos;
	float3 L = normalize(float3(5.0, 5.0, -5.0)); // Directional light
    
	float t = 0.01;
	float glow = 0.0; // Bloom accumulator
	float3 glowColor = float3(0.4, 0.7, 1.0);
	for (int i = 0; i < 256; i++)
	{
		float3 currPos = rayOrigin + rayDir * t;
		float d = Julia3DDE(currPos);
		float threshold = 0.0005 * t;
		glow += exp(-d * 5.0) * 0.05;
		if (d < threshold)
		{
			float3 N = GetNormal(currPos, t);
			float3 V = -rayDir;
			
			// Ambient occlusion 
			float ao = saturate(1.0 - (float(i) / 128.0));
			ao = pow(ao, 2.0);
            // shading with Ambient Occlusion and Rim lighting
			float diff = max(dot(N, L), 0.0) * 0.8 + 0.2;
			float rim = pow(1.0 - saturate(dot(N, V)), 4.0);
			
			float3 baseColor = lerp(float3(0.35, 0.1, 0.3490), float3(0.7, 0.6, 1.0), saturate(orbitTrap * 2.0));
			float3 finalColor = (baseColor * diff + rim * 0.4) * ao;
			
			return float4(finalColor + (glowColor * glow * 0.2), 1.0);
		}
		if (t > 10.0)
			break;
		t += d * 0.75f;
	}

	return float4(0.02, 0.02, 0.05, 1.0); // Space background
}