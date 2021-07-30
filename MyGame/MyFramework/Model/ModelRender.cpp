#include "MyFramework.h"
#include "ModelRender.h"

ModelRender::ModelRender(Shader* shader)
	: shader(shader)
{
	model = new Model();
	transform = new Transform(shader);
}

ModelRender::~ModelRender()
{
	SafeDelete(model);
	SafeDelete(transform);
}

void ModelRender::Update()
{
	if (bRead == true) // 모델 로드가 되있는지 확인하는 조건문
	{
		bRead = false;

		for (ModelMesh* mesh : model->Meshes())
			mesh->SetShader(shader);

		UpdateTransform();
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->Update();
}

void ModelRender::Render()
{
	for (ModelMesh* mesh : model->Meshes()) // 렌더링의 기준은 Mesh이며 모델이 글질 Trnasform을 설정
	{
		mesh->SetTransform(transform);
		mesh->Render();
	}
}

void ModelRender::ReadMesh(wstring file)
{
	model->ReadMesh(file); // 파일로부터 매쉬를 읽어오는 과정
}

void ModelRender::ReadMaterial(wstring file)
{
	model->ReadMaterial(file);

	bRead = true;
}

void ModelRender::Pass(UINT pass)
{
	for (ModelMesh* mesh : model->Meshes())
		mesh->Pass(pass);
}

void ModelRender::UpdateTransform(ModelBone* bone, Matrix& matrix)
{
	if (bone != NULL) // 본이 있다면 본의 매트릭스를 matrix로 할당
		UpdateBones(bone, matrix);

	for (UINT i = 0; i < model->BoneCount(); i++)
	{
		ModelBone* bone = model->BoneByIndex(i);
		transforms[i] = bone->Transform();
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->Transforms(transforms); // 자기가 랜더링할 쉐이더를 밀어주는 과정
}

void ModelRender::UpdateBones(ModelBone* bone, Matrix& matrix)
{

}

