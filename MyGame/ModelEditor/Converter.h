#pragma once

class Converter
{
public:
	Converter();
	~Converter();

	void ReadFile(wstring file);

public:
	void ExportMesh(wstring savePath); // �޽� ����

private:
	void ReadBoneData(aiNode* node, int index, int parent); // ��ȸ�� �ϸ鼭 ��͸� �ҷ��� �Լ�
	void ReadMeshData(aiNode* node, int bone); // ��͸� �ϸ鼭 Mesh �����͸� �ҷ����� �Լ�
	void ReadSkinData(); // �ִϸ��̼ǿ� �ʿ��� Blend�����͸� �ҷ����� �Լ�
	void WriteMeshData(wstring savePath); // ���� �޽��� �о �츮�� ���ϴ� ������ �����ߴٸ� �츮�� ���ϴ� ���Ϸ� �����ϴ� �Լ�

public:
	void ExportMaterial(wstring savePath, bool bOverwrite = true); // ���׸����� �����̳ʳ� �����ڸ��� ������ �� �ִ� �κ��� ���� �� �ֱ� ������ XML���Ϸ� �����ϰ� ���� ���� �������� ����

private:
	void ReadMaterialData();
	void WriteMaterialData(wstring savePath);
	string WriteTexture(string saveFolder, string file);

public:
	void ClipList(vector<wstring>* list); // ���� �������� �ִ� Ŭ���� �ƴ� �Ѱ��� fbx���Ͽ� Ŭ���� ���� ��츦 ����ϱ����� �Լ�
	void ExportAnimClip(UINT index, wstring savePath);

private:
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyframeData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);
	void WriteClipData(struct asClip* clip, wstring savePath);

private:
	wstring file;

	Assimp::Importer* importer;
	const aiScene* scene;

	vector<struct asBone*> bones; // �� ������ ������ �ִ� ����
	vector<struct asMesh*> meshes; // �޽� ������ ������ �ִ� ����
	vector<struct asMaterial*> materials; // ���͸��� ������ ������ �ִ� ����
};