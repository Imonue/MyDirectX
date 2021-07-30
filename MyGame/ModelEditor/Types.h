#pragma once
#include "stdafx.h"

struct asBone // �� ������ ������ �ִ� ����ü
{
	int Index;
	string Name;

	int Parent;
	Matrix Transform; // ���� ���
};


/*
* ���� �޽ô� 1:1�� �Ϲ��������� �޽ð� ���� ��쵵 ����
*/

struct asMesh // �޽� ������ ������ �ִ� ����ü
{
	string Name;
	int BoneIndex;

	aiMesh* Mesh;

	string MaterialName; // ������ ���� �̸�

	vector<Model::ModelVertex> Vertices;
	vector<UINT> Indices;
};

struct asMaterial
{
	string Name; // �޽��� �̸��� ��ġ�ϴ� �̸��� �����ϴ� ����

	Color Ambient;
	Color Diffuse;
	Color Specular;
	Color Emissive;

	string DiffuseFile; // �츮�� ������ ������ �ؽ�ó ����
	string SpecularFile;
	string NormalFile;
};


struct asBlendWeight // �ҷ��� �����͸� ���������� ���� ����ü
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

struct asBoneWeights // �ҷ��� �����͸� �����ϱ� ���� ����ü
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
			if (boneWeights > it->second) // �� ����ġ�� ������ �־��� �ͺ��� ������쿡 �ڸ��� �߰�
			{
				BoneWeights.insert(it, Pair(boneIndex, boneWeights));
				bInsert = true;

				break;
			}

			it++;
		} // while(it)

		if (bInsert == false) // �ƴ� ��� ������ ���� �ǵڿ� �߰������� ����ġ���� ����
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

	void Normalize() // ũ�⸦ 1�� ������ִ� �Լ�
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

// Ŭ���� ������ �ִ� Ű �������� ���� ������
struct asKeyframeData
{
	float Time; // �ش� �����Ͱ� �� �����ӿ� ��ġ���ִ����� ���� ex) 0������, 15������

	Vector3 Scale; // �ش� Ű �����ӿ��� ������ ���� ������ ��
	Quaternion Rotation; // �ش� Ű �����ӿ��� ������ ���� ȸ�� ��
	Vector3 Translation; // �ش� Ű �����ӿ��� ������ ���� ��ġ ��
};

// �����Ͱ� ������ ����ü
struct asKeyframe
{
	string BoneName;
	vector<asKeyframeData> Transforms;
};

// �����Ͱ� ������ ����ü
struct asClip
{
	string Name;

	UINT FrameCount;
	float FrameRate;
	float Duration;

	vector<asKeyframe*> Keyframes;
};

//aniNode�� ���� Ű������ ����
//���Ϸ� ����� ������
struct asClipNode
{
	aiString Name;
	vector<asKeyframeData> Keyframe; // �ش� Ŭ���� �������ִ� Ű������ fps�� ������ �ִ� ����
};