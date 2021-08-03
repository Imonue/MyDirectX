#pragma once
#define MAX_MESH_INSTANCE 500

class MeshRender
{
public:
	MeshRender(Shader* shader, Mesh* mesh); // 객체가 그려질 쉐이더와 그려질 메시가 인자
	~MeshRender();

	Mesh* GetMesh() { return mesh; }

	void Update();
	void Render();

	void Pass(UINT val) { mesh->Pass(val); }

	Transform* AddTransform(); // 트랜스폼의 추가를 위한 함수로 그릴 개수만큼 함수가 호출
	Transform* GetTransform(UINT index) { return transforms[index]; } // 트랜스폼을 받아올 수 있는 함수
	void UpdateTransforms(); // transforms에 있는 값들을 worlds로 복사하고 바로 GPU쪽으로 복사해주는 함수

private:
	Mesh* mesh;

	vector<Transform*> transforms; // 외부에서 그릴 개수만큼 벡터에 푸시, 트랜스폼즈의 개수가 인스턴스의 개수
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;
};