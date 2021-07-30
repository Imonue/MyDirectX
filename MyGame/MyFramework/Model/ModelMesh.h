#pragma once

class ModelBone
{
public:
	friend class Model; // ModelBone이 Model에서 열어졌다라고 이해

private: // Model에서만 생성할 수 있도록 private으로 지정
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
	int index; // 파일을 불러올 때 입력
	wstring name; // 파일을 불러올 떄 입력

	int parentIndex; // 부모의 인덱스
	ModelBone* parent; // 부모의 Bone

	Matrix transform;
	vector<ModelBone*> childs; // 자식 Bone들
};

///////////////////////////////////////////////////////////////////////////////

/*
* 드로잉하는데 있어서 주체가 되는 클래스
*/
class ModelMesh
{
public:
	friend class Model; // ModelMesh가 Model에서 열어졌다라고 이해

private:
	ModelMesh();
	~ModelMesh();

	void Binding(Model* model); // 연결하는 함수

public:
	void Pass(UINT val) { pass = val; }
	void SetShader(Shader* shader);

	void Update();
	void Render();

	wstring Name() { return name; }

	int BoneIndex() { return boneIndex; }
	class ModelBone* Bone() { return bone; }

	void Transforms(Matrix* transforms); // BoneDesc의 Transforms를 복사해주기 위한 함수
	void SetTransform(Transform* transform); // 모델의 위치를 움직일 함수

	void TransformsSRV(ID3D11ShaderResourceView* srv) { transformsSRV = srv; }

private:
	struct BoneDesc // 본 정보를 넘길 때 사용하는 구조체
	{
		Matrix Transforms[MAX_MODEL_TRANSFORMS]; // 전체 본을 배열로 만들어서 CBuffer로 전달

		UINT Index; // 현재 그려질 매쉬가 참조할 본의 번호가 같이 Cbuffer로 전달
		float Padding[3]; // 16byte를 맞춰야하므로 추가하는 변수
	} boneDesc;

private:
	wstring name;

	Shader* shader;
	UINT pass = 0;

	Transform* transform = NULL;
	PerFrame* perFrame = NULL;

	wstring materialName = L"";
	Material* material;


	int boneIndex; // 참조할 Bone index
	class ModelBone* bone; // binding 해서 연결한 Bone

	VertexBuffer* vertexBuffer; // 정점 버퍼
	UINT vertexCount; // 정점 개수
	Model::ModelVertex* vertices; // 정점 배열

	IndexBuffer* indexBuffer; // 인덱스 버퍼
	UINT indexCount; // 인덱스 개수
	UINT* indices; // 인덱스 배열

	ConstantBuffer* boneBuffer; // BoneDesc를 전달받을 Cbuffer
	ID3DX11EffectConstantBuffer* sBoneBuffer;

	ID3D11ShaderResourceView* transformsSRV = NULL;
	ID3DX11EffectShaderResourceVariable* sTransformsSRV;
};