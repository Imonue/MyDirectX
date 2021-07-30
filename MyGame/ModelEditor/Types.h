#pragma once
#include "stdafx.h"

struct asBone // 본 정보를 가지고 있는 구조체
{
	int Index;
	string Name;

	int Parent;
	Matrix Transform; // 본의 행렬
};


/*
* 본과 메시는 1:1이 일반적이지만 메시가 없는 경우도 존재
*/

struct asMesh // 메시 정보를 가지고 있는 구조체
{
	string Name;
	int BoneIndex;

	aiMesh* Mesh;

	string MaterialName; // 참조할 재질 이름

	vector<Model::ModelVertex> Vertices;
	vector<UINT> Indices;
};

struct asMaterial
{
	string Name; // 메시의 이름과 일치하는 이름을 저장하는 변수

	Color Ambient;
	Color Diffuse;
	Color Specular;
	Color Emissive;

	string DiffuseFile; // 우리가 실제로 보여질 텍스처 파일
	string SpecularFile;
	string NormalFile;
};


struct asBlendWeight // 불러온 데이터를 최종적으로 담을 구조체
{
	Vector4 Indices = Vector4(0, 0, 0, 0);
	Vector4 Weights = Vector4(0, 0, 0, 0);

	void Set(UINT index, UINT boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: Indices.x = i; Weights.x = w; break;
		case 1: Indices.y = i; Weights.y = w; break;
		case 2: Indices.z = i; Weights.z = w; break;
		case 3: Indices.w = i; Weights.w = w; break;
		}
	}
};

struct asBoneWeights // 불러온 데이터를 정리하기 위한 구조체
{
private:
	typedef pair<int, float> Pair;
	vector<Pair> BoneWeights;

public:
	void AddWeights(UINT boneIndex, float boneWeights)
	{
		if (boneWeights <= 0.0f) return;

		bool bInsert = false;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (boneWeights > it->second) // 본 가중치가 이전에 있었던 것보다 작을경우에 자리에 추가
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bInsert = true;

				break;
			}

			it++;
		} // while(it)

		if (bInsert == false) // 아닐 경우 이전에 가장 맨뒤에 추가함으로 가중치별로 정렬
			BoneWeights.push_back(Pair(boneIndex, boneWeights));
	}

	void GetBlendWeights(asBlendWeight& blendWeights)
	{
		for (UINT i = 0; i < BoneWeights.size(); i++)
		{
			if (i >= 4) return;

			blendWeights.Set(i, BoneWeights[i].first, BoneWeights[i].second);
		}
	}

	void Normalize() // 크기를 1로 만들어주는 함수
	{
		float totalWeight = 0.0f;

		int i = 0;
		vector<Pair>::iterator it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			if (i < 4)
			{
				totalWeight += it->second;
				i++; it++;
			}
			else
				it = BoneWeights.erase(it);
		}

		float scale = 1.0f / totalWeight;

		it = BoneWeights.begin();
		while (it != BoneWeights.end())
		{
			it->second *= scale;
			it++;
		}
	}
};

// 클립이 가지고 있는 키 프레임을 담을 데이터
struct asKeyframeData
{
	float Time; // 해당 데이터가 몇 프레임에 위치해있는지를 지정 ex) 0프레임, 15프레임

	Vector3 Scale; // 해당 키 프레임에서 가지고 있을 스케일 값
	Quaternion Rotation; // 해당 키 프레임에서 가지고 있을 회전 값
	Vector3 Translation; // 해당 키 프레임에서 가지고 있을 위치 값
};

// 데이터가 씌여질 구조체
struct asKeyframe
{
	string BoneName;
	vector<asKeyframeData> Transforms;
};

// 데이터가 씌여질 구조체
struct asClip
{
	string Name;

	UINT FrameCount;
	float FrameRate;
	float Duration;

	vector<asKeyframe*> Keyframes;
};

//aniNode의 원본 키프레임 저장
//파일로 저장될 데이터
struct asClipNode
{
	aiString Name;
	vector<asKeyframeData> Keyframe; // 해당 클립이 가지고있는 키프레임 fps를 가지고 있는 벡터
};