#pragma once

/*
* ������ ���� ��ϰ� �������ϴ� Ŭ����
*/
class ModelRender
{
public:
	ModelRender(Shader* shader);
	~ModelRender();

	void Update(); // ������Ʈ �Լ�
	void Render(); // ���� �Լ�

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);

	Transform* GetTransform() { return transform; }
	Model* GetModel() { return model; }

	void Pass(UINT pass); // �Ž��� ���ƴٴϸ鼭 �н��� ����

	void UpdateTransform(ModelBone* bone = NULL, Matrix& matrix = Matrix()); // ����Ŀ� ���� ��ü�� Ʈ�������� ������Ʈ

private:
	void UpdateBones(ModelBone* bone, Matrix& matrix); // �� ����� ȸ���� ���� ������ �Լ�

private:
	bool bRead = false; // ���� �ҷ������� Ȯ���ϴ� ����, ���� ����� ó���� �ʿ�

	Shader* shader;
	Model* model; // �� ��ü
	Transform* transform; // �� Ŭ������ ���α� ���� Ʈ������

	Matrix transforms[MAX_MODEL_TRANSFORMS]; // �� �迭 ��ü�� ������ ���� �迭
};