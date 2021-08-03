#pragma once
#define MAX_MESH_INSTANCE 500

class MeshRender
{
public:
	MeshRender(Shader* shader, Mesh* mesh); // ��ü�� �׷��� ���̴��� �׷��� �޽ð� ����
	~MeshRender();

	Mesh* GetMesh() { return mesh; }

	void Update();
	void Render();

	void Pass(UINT val) { mesh->Pass(val); }

	Transform* AddTransform(); // Ʈ�������� �߰��� ���� �Լ��� �׸� ������ŭ �Լ��� ȣ��
	Transform* GetTransform(UINT index) { return transforms[index]; } // Ʈ�������� �޾ƿ� �� �ִ� �Լ�
	void UpdateTransforms(); // transforms�� �ִ� ������ worlds�� �����ϰ� �ٷ� GPU������ �������ִ� �Լ�

private:
	Mesh* mesh;

	vector<Transform*> transforms; // �ܺο��� �׸� ������ŭ ���Ϳ� Ǫ��, Ʈ���������� ������ �ν��Ͻ��� ����
	Matrix worlds[MAX_MESH_INSTANCE];

	VertexBuffer* instanceBuffer;
};