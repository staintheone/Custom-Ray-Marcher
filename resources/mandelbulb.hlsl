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
cbuffer MandelbulbSettings : register(b3)
{
    float Power;
    float fractalLOD;
    float bailout;
    float epsilonVal;
    float surfThreshold;
    float maxIterations;
    float stepMultiplier;
    float glowComponent;

};
cbuffer fractalColor : register(b4)
{
    float4 colorComponentA;
    float4 colorComponentB;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
// Global variable to store color logic
static float orbitTrap = 0.0;
float MandelbulbDE(float3 p)
{
    float3 z = p;
    float dr = 1.0;
    float r = 0.0;
    float oTrap = 1e10;

    for (int i = 0; i < fractalLOD; i++)
    {
        r = length(z);
        if (r > bailout)
            break; // Bailout

        // Keep track of the running derivative
        dr = pow(r, Power - 1.0) * Power * dr + 1.0;

        // Convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan2(z.y, z.x);
        
        //  Scale and rotate
        float zr = pow(r, Power);
        theta *= Power;
        phi *= Power;

        // Convert back to cartesian
        z = zr * float3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += p;
        
        oTrap = min(oTrap, abs(z.z));
    }
    
    orbitTrap = oTrap;
    
    // The Distance Estimate Formula: 0.5 * log(r) * r / dr
    // This provides an accurate distance to surface.
    return 0.5 * log(r) * r / dr;
}
//Calculate the surface normal using a gradient
float3 GetNormal(float3 p, float t)
{
    // epsilon value
    float e = epsilonVal * t;
    
    // sampling 4 points in a tetrahedron shape (more stable than the axis-aligned approach)
    float2 h = float2(1.0, -1.0) * 0.5773;
    return normalize(
        h.xyy * MandelbulbDE(p + h.xyy * e) +
        h.yyx * MandelbulbDE(p + h.yyx * e) +
        h.yxy * MandelbulbDE(p + h.yxy * e) +
        h.xxx * MandelbulbDE(p + h.xxx * e)
    );
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
    float3 rayDir = normalize(mul((float3x3) invView, float3(p, 1.5)));
    float3 rayOrigin = cameraPos;
	
    float glow = 0.0; // Bloom accumulator
    float3 glowColor = float3(0.4, 0.7, 1.0); // Neon Blue Glow
    
	//Ray Marching Loop
    float t = 0.01; // Total distance traveled
    [loop] // Hint to compiler for performance
    for (int i = 0; i < maxIterations; i++)
    {
        float3 currPos = rayOrigin + rayDir * t;
        float d = MandelbulbDE(currPos);
        glow += exp(-d * 5.0) * glowComponent;
 
        if (d < surfThreshold * t)
        {
            // -- PHONG LIGHTING CALCULATIONS --
            float3 N = GetNormal(currPos, t); // Surface Normal
            float3 L = normalize(float3(3.0, 3.0, -3.0)); // Light Position (World-based)
            float3 V = -rayDir; // View Direction (towards camera)
            float3 R = reflect(-L, N); // Reflected Light vector
            
            // Base Color using Orbit Trap (Purple to Gold)
            float3 colorA = colorComponentA.rgb; // The glow around edges 
            float3 colorB = colorComponentB.rgb; // the color of fractal itself 
            float3 baseColor = lerp(colorA, colorB, saturate(orbitTrap * 3.0));
            
            int steps = 0;
            // Ambient Occlusion (darkens the deep cavities of the fractal)
            float ao = 1.0 - (float(i) / maxIterations);
            
            // Rim Lighting (Highlights the silhouette)
            float rim = pow(1.0 - max(dot(N, V), 0.0), 4.0);

            // Lighting -> Using baseColor for diffuse
            float dotNL = max(dot(N, L), 0.0);
            float3 diffuse = baseColor * dotNL;
            float3 specular = float3(1.0, 1.0, 1.0) * pow(max(dot(R, V), 0.0), 64.0);

            // Combining all effects
            float3 finalColor = (diffuse + (specular * 0.5) + (rim * 0.3)) * ao;
            float3 ambient = colorA * 0.2 * 1.5;

            // Final Color
            return float4(finalColor + (glowColor * glow * ambient), 1.0) * 1.3f;
        }
        
        if (t > 10.0)
            break; // Missed everything, stop marching
        t += d * stepMultiplier;
    }

    // Background
    return float4(0.05, 0.05, 0.1, 1.0);
    //return float4(rayDir * 0.5 + 0.5, 1.0);
}