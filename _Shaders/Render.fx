struct VertexMesh
{
    float4 Position : Position;
    float2 Uv : Uv;
    float3 Normal : Normal;
    
    matrix Transform : InstTransform;
};

/////////////////////////////////////////////////////////////////////////////////

#define VS_GENERATE \
output.oPosition = input.Position; \
\
output.Position = WorldPosition(input.Position); \
output.Position = ViewProjection(output.Position); \
\
output.Normal = WorldNormal(input.Normal); \
\
output.Uv = input.Uv;

/////////////////////////////////////////////////////////////////////////////////

// 월드 계산 함수
void SetMeshWorld(inout matrix world, VertexMesh input)
{
    world = input.Transform;
}

MeshOutput VS_Mesh(VertexMesh input)
{
    MeshOutput output;
    
    SetMeshWorld(World, input);
    VS_GENERATE
    
    return output;
}
