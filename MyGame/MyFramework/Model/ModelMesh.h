#pragma once

class ModelBone
{
public:
	friend class Model; // ModelBone�� Model���� �������ٶ�� ����

private: // Model������ ������ �� �ֵ��� private���� ����
	ModelBone();
	~ModelBone();

public:
	int Index() { return index; }

	int ParentIndex() { return parentIndex; }
	ModelBone* Parent() { return parent; }

	wstring Name() { return name; }

	Matrix& Transform() { return transform; }
	void Transform(Matrix& matrix) { transform = matrix; }

	vector<ModelBone*>& Childs() { return childs; }

private:
	int index; // ������ �ҷ��� �� �Է�
	wstring name; // ������ �ҷ��� �� �Է�

	int parentIndex; // �θ��� �ε���
	ModelBone* parent; // �θ��� Bone

	Matrix transform;
	vector<ModelBone*> childs; // �ڽ� Bone��
};

///////////////////////////////////////////////////////////////////////////////

/*
* ������ϴµ� �־ ��ü�� �Ǵ� Ŭ����
*/
class ModelMesh
{
public:
	friend class Model; // ModelMesh�� Model���� �������ٶ�� ����

private:
	ModelMesh();
	~ModelMesh();

	void Binding(Model* model); // �����ϴ� �Լ�

public:
	void Pass(UINT val) { pass = val; }
	void SetShader(Shader* shader);

	void Update();
	void Render();

	wstring Name() { return name; }

	int BoneIndex() { return boneIndex; }
	class ModelBone* Bone() { return bone; }

	void Transforms(Matrix* transforms); // BoneDesc�� Transforms�� �������ֱ� ���� �Լ�
	void SetTransform(Transform* transform); // ���� ��ġ�� ������ �Լ�

	void TransformsSRV(ID3D11ShaderResourceView* srv) { transformsSRV = srv; }

private:
	struct BoneDesc // �� ������ �ѱ� �� ����ϴ� ����ü
	{
		Matrix Transforms[MAX_MODEL_TRANSFORMS]; // ��ü ���� �迭�� ���� CBuffer�� ����

		UINT Index; // ���� �׷��� �Ž��� ������ ���� ��ȣ�� ���� Cbuffer�� ����
		float Padding[3]; // 16byte�� ������ϹǷ� �߰��ϴ� ����
	} boneDesc;

private:
	wstring name;

	Shader* shader;
	UINT pass = 0;

	Transform* transform = NULL;
	PerFrame* perFrame = NULL;

	wstring materialName = L"";
	Material* material;


	int boneIndex; // ������ Bone index
	class ModelBone* bone; // binding �ؼ� ������ Bone

	VertexBuffer* vertexBuffer; // ���� ����
	UINT vertexCount; // ���� ����
	Model::ModelVertex* vertices; // ���� �迭

	IndexBuffer* indexBuffer; // �ε��� ����
	UINT indexCount; // �ε��� ����
	UINT* indices; // �ε��� �迭

	ConstantBuffer* boneBuffer; // BoneDesc�� ���޹��� Cbuffer
	ID3DX11EffectConstantBuffer* sBoneBuffer;

	ID3D11ShaderResourceView* transformsSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
};