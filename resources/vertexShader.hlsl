///// Vertex Shader /////
// The vertex shader is only used to generate a single quad covering the entire scree, 
// the execution of vertex shader is reqiured for the execution of pixel shader where all the ray marching happens.

// Global variables
struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Vert shader entry point
VS_OUTPUT VS_Main(uint vID : SV_VertexID)
{
	VS_OUTPUT output;
	
	// This bitwise math creates 3 vertices: (0,0), (2,0), (0,2)
	output.uv = float2((vID << 1) & 2, vID & 2);
	
	// This converts the UVs into Clip Space (NDC):
    // ID 0 -> (-1, 1)  (Top Left)
    // ID 1 -> ( 3, 1)  (Way out right)
    // ID 2 -> (-1,-3)  (Way out bottom)
    // The resulting giant triangle perfectly covers the -1 to 1 screen quad.
	output.position = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

	return output;
}