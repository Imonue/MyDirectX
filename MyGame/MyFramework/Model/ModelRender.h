#pragma once

/*
* 실제로 모델을 운영하고 렌더링하는 클래스
*/
class ModelRender
{
public:
	ModelRender(Shader* shader);
	~ModelRender();

	void Update(); // 업데이트 함수
	void Render(); // 랜더 함수

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);

	Transform* GetTransform() { return transform; }
	Model* GetModel() { return model; }

	void Pass(UINT pass); // 매쉬를 돌아다니면서 패스를 설정

	void UpdateTransform(ModelBone* bone = NULL, Matrix& matrix = Matrix()); // 본행렬에 대한 전체의 트랜스폼을 업데이트

private:
	void UpdateBones(ModelBone* bone, Matrix& matrix); // 본 행렬의 회전을 위해 설정한 함수

private:
	bool bRead = false; // 모델이 불려졌는지 확인하는 변수, 최초 리드시 처리시 필요

	Shader* shader;
	Model* model; // 모델 객체
	Transform* transform; // 모델 클래스를 감싸기 위한 트랜스폼

	Matrix transforms[MAX_MODEL_TRANSFORMS]; // 본 배열 전체를 가지고 있을 배열
};