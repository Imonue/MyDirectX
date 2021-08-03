#include "MyFramework.h"
#include "MeshRender.h"

MeshRender::MeshRender(Shader* shader, Mesh* mesh)
	: mesh(mesh)
{
	Pass(0); // Pass는 기본값 0
	mesh->SetShader(shader); // 쉐이더 할당

	for (UINT i = 0; i < MAX_MESH_INSTANCE; i++)
		D3DXMatrixIdentity(&worlds[i]); // Matrix들을 정규화
	// 마지막 인자는 CPU Write를 켜주기위한 인자로 SRT가 변경되었을 때 CPU가 GPU로 복사 할 수 있도록 활성
	instanceBuffer = new VertexBuffer(worlds, MAX_MESH_INSTANCE, sizeof(Matrix), 1, true);
}

MeshRender::~MeshRender()
{
	for (Transform* transform : transforms)
		SafeDelete(transform);

	SafeDelete(instanceBuffer);
	SafeDelete(mesh);
}

void MeshRender::Update()
{
	mesh->Update();
}

void MeshRender::Render()
{
	instanceBuffer->Render();

	mesh->Render(transforms.size()); // 메시가 실제로 그려질 개수
}

Transform* MeshRender::AddTransform()
{
	Transform* transform = new Transform();
	transforms.push_back(transform);
	return transform;
}

void MeshRender::UpdateTransforms()
{
	for (UINT i = 0; i < transforms.size(); i++)
		memcpy(worlds[i], transforms[i]->World(), sizeof(Matrix)); // transforms에 있던 정보를 worlds에 복사

	D3D11_MAPPED_SUBRESOURCE subResource; // Map과 subResource를 이용해서 GPU로 복사
	D3D::GetDC()->Map(instanceBuffer->Buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	{
		memcpy(subResource.pData, worlds, sizeof(Matrix) * MAX_MESH_INSTANCE);
	}
	D3D::GetDC()->Unmap(instanceBuffer->Buffer(), 0);
}
