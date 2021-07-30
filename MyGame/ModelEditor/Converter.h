#pragma once

class Converter
{
public:
	Converter();
	~Converter();

	void ReadFile(wstring file);

public:
	void ExportMesh(wstring savePath); // 메쉬 정보

private:
	void ReadBoneData(aiNode* node, int index, int parent); // 순회를 하면서 재귀를 불러올 함수
	void ReadMeshData(aiNode* node, int bone); // 재귀를 하면서 Mesh 데이터를 불러오는 함수
	void ReadSkinData(); // 애니메이션에 필요한 Blend데이터를 불러오는 함수
	void WriteMeshData(wstring savePath); // 본과 메쉬를 읽어서 우리가 원하는 변수로 저장했다면 우리가 원하는 파일로 저장하는 함수

public:
	void ExportMaterial(wstring savePath, bool bOverwrite = true); // 마테리얼은 디자이너나 개발자마다 조정할 수 있는 부분이 생길 수 있기 떄문에 XML파일로 저장하고 덮어 씌울 것인지를 지정

private:
	void ReadMaterialData();
	void WriteMaterialData(wstring savePath);
	string WriteTexture(string saveFolder, string file);

public:
	void ClipList(vector<wstring>* list); // 따로 나누어져 있는 클립이 아닌 한개의 fbx파일에 클립이 있을 경우를 대비하기위한 함수
	void ExportAnimClip(UINT index, wstring savePath);

private:
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyframeData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);
	void WriteClipData(struct asClip* clip, wstring savePath);

private:
	wstring file;

	Assimp::Importer* importer;
	const aiScene* scene;

	vector<struct asBone*> bones; // 본 정보를 가지고 있는 벡터
	vector<struct asMesh*> meshes; // 메시 정보를 가지고 있는 벡터
	vector<struct asMaterial*> materials; // 매터리얼 정보를 가지고 있는 벡터
};