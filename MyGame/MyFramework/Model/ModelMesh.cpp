#include "MyFramework.h"
#include "ModelMesh.h"

ModelBone::ModelBone()
{

}

ModelBone::~ModelBone()
{

}

///////////////////////////////////////////////////////////////////////////////

ModelMesh::ModelMesh()
{
	boneBuffer = new ConstantBuffer(&boneDesc, sizeof(BoneDesc));
}

ModelMesh::~ModelMesh()
{
	SafeDelete(transform);
	SafeDelete(perFrame);

	SafeDelete(material);

	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	SafeDelete(vertexBuffer);
	SafeDelete(indexBuffer);

	SafeDelete(boneBuffer);
}

void ModelMesh::Binding(Model* model)
{
	vertexBuffer = new VertexBuffer(vertices, vertexCount, sizeof(Model::ModelVertex));
	indexBuffer = new IndexBuffer(indices, indexCount);

	Material* srcMaterial = model->MaterialByName(materialName);

	material = new Material();
	material->CopyFrom(srcMaterial);
}

void ModelMesh::SetShader(Shader* shader)
{
	/*
	* 쉐이더가 바뀌게 될 경우 transform과 perFrame은 초기에 생성할 때 쉐이더를 받으므로 삭제하고 다시 할당후 생성
	* sBoneBuffer를 쉐이더로부터 받아온다
	*/
	this->shader = shader;

	SafeDelete(transform);
	transform = new Transform(shader);

	SafeDelete(perFrame);
	perFrame = new PerFrame(shader);

	sBoneBuffer = shader->AsConstantBuffer("CB_Bone");

	material->SetShader(shader);
	sTransformsSRV = shader->AsSRV("TransformsMap");
}

void ModelMesh::Update()
{
	/*
	* BoneDesc로부터 모델이 참조하고 있는 배열의 인덱스로부터 가져온 값과 월드를 결합해서 최종 위치 도출
	*/
	boneDesc.Index = boneIndex;

	perFrame->Update();
	transform->Update();
}

void ModelMesh::Render()
{
	/*
	* 
	*/
	boneBuffer->Render();
	sBoneBuffer->SetConstantBuffer(boneBuffer->Buffer());

	perFrame->Render();
	transform->Render();
	material->Render();

	vertexBuffer->Render();
	indexBuffer->Render();

	if (transformsSRV != NULL)
		sTransformsSRV->SetResource(transformsSRV);

	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->DrawIndexed(0, pass, indexCount);
}

void ModelMesh::Transforms(Matrix* transforms)
{
	/*
	* boneDesc의 Transform 배열을 복사하는 함수
	*/
	memcpy(boneDesc.Transforms, transforms, sizeof(Matrix) * MAX_MODEL_TRANSFORMS);
}

void ModelMesh::SetTransform(Transform* transform)
{
	/*
	* ModelMesh의 Transform을 복사하는 함수
	*/
	this->transform->Set(transform);
}


