#include "Global.fx"
#include "Render.fx"

float3 Direction = float3(-1, -1, +1);


float4 PS(MeshOutput input) : SV_Target
{
    float3 normal = normalize(input.Normal);
    float3 light = -Direction;
    
    return DiffuseMap.Sample(LinearSampler, input.Uv) * dot(light, normal);
}


technique11 T0
{
    P_VP(P0, VS_Mesh, PS)
}