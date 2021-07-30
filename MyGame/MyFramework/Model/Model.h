#pragma once
#define MAX_MODEL_TRANSFORMS 250 
#define MAX_MODEL_KEYFRAMES 500

//본의 전체 개수는 250개로 설정은 사람의 관절이 250개가 넘지 않기 때문
//모델에서 데이터를 불러와서 랜더링
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

	UINT BoneCount() { return bones.size(); } // 본의 개수를 반환하는 함수
	vector<ModelBone*>& Bones() { return bones; } // 본 전체를 반환하는 함수
	ModelBone* BoneByIndex(UINT index) { return bones[index]; } // 본의 인덱스로 검색하는 함수
	ModelBone* BoneByName(wstring name); // 본의 이름으로 검색하는 함수

	UINT MeshCount() { return meshes.size(); }
	vector<ModelMesh*>& Meshes() { return meshes; }
	ModelMesh* MeshByIndex(UINT index) { return meshes[index]; }
	ModelMesh* MeshByName(wstring name);

	UINT MaterialCount() { return materials.size(); }
	vector<Material*>& Materials() { return materials; }
	Material* MaterialByIndex(UINT index) { return materials[index]; }
	Material* MaterialByName(wstring name);

public:
	void ReadMesh(wstring file); // 파일에서 데이터를 읽어오는 함수
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

	UINT ClipCount() { return clips.size(); }
	vector<ModelClip*>& Clips() { return clips; }
	ModelClip* ClipByIndex(UINT index) { return clips[index]; }
	ModelClip* ClipByName(wstring name);

private:
	void BindBone(); // 본과 연결하는 함수
	void BindMesh(); // 메쉬와 연결하는 함수

private:
	ModelBone* root; // 순회를 하기 위한 최상위 부모
	vector<ModelBone*> bones; // 본 정보를 담고 있는 배열
	vector<ModelMesh*> meshes; // 메쉬 정보를 담고 있는 배열
	vector<Material*> materials;
	vector<ModelClip*> clips;
};