#pragma once
#define MAX_MODEL_TRANSFORMS 250 
#define MAX_MODEL_KEYFRAMES 500

//���� ��ü ������ 250���� ������ ����� ������ 250���� ���� �ʱ� ����
//�𵨿��� �����͸� �ҷ��ͼ� ������
class ModelBone;
class ModelMesh;
class ModelClip;

class Model
{
public:
	typedef VertexTextureNormalTangentBlend ModelVertex;

public:
	Model();
	~Model();

	UINT BoneCount() { return bones.size(); } // ���� ������ ��ȯ�ϴ� �Լ�
	vector<ModelBone*>& Bones() { return bones; } // �� ��ü�� ��ȯ�ϴ� �Լ�
	ModelBone* BoneByIndex(UINT index) { return bones[index]; } // ���� �ε����� �˻��ϴ� �Լ�
	ModelBone* BoneByName(wstring name); // ���� �̸����� �˻��ϴ� �Լ�

	UINT MeshCount() { return meshes.size(); }
	vector<ModelMesh*>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	UINT MaterialCount() { return materials.size(); }
	vector<Material*>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

public:
	void ReadMesh(wstring file); // ���Ͽ��� �����͸� �о���� �Լ�
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

	UINT ClipCount() { return clips.size(); }
	vector<ModelClip*>& Clips() { return clips; }
	ModelClip* ClipByIndex(UINT index) { return clips[index]; }
	ModelClip* ClipByName(wstring name);

private:
	void BindBone(); // ���� �����ϴ� �Լ�
	void BindMesh(); // �޽��� �����ϴ� �Լ�

private:
	ModelBone* root; // ��ȸ�� �ϱ� ���� �ֻ��� �θ�
	vector<ModelBone*> bones; // �� ������ ��� �ִ� �迭
	vector<ModelMesh*> meshes; // �޽� ������ ��� �ִ� �迭
	vector<Material*> materials;
	vector<ModelClip*> clips;
};