#include "MyFramework.h"
#include "MeshSphere.h"

MeshSphere::MeshSphere(float radius, UINT stackCount, UINT sliceCount)
	: radius(radius), stackCount(stackCount), sliceCount(sliceCount)
{

}

MeshSphere::~MeshSphere()
{

}

void MeshSphere::Create()
{
	/*
	* 삼각함수, 삼각함수의 항등식, 구면 좌표계
	* 
	*/
	vector<MeshVertex> v;
	v.push_back(MeshVertex(0, radius, 0, 0, 0, 0, 1, 0)); // 구 맨 위에 점으로 노멀은 수직방향


	float phiStep = Math::PI / stackCount; // p를 그려나갈 크기, 파이(180)
	float thetaStep = Math::PI * 2.0f / sliceCount; // 360 / sliceCOunt

	// 윗점을 하나 더 그린상태고 아랫점을 하나 더 그릴 예정이므로 -1까지 반복문
	for (UINT i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (UINT k = 0; k <= sliceCount; k++)
		{
			float theta = k * thetaStep;

			Vector3 p = Vector3
			(
				(radius * sinf(phi)  * cosf(theta)),
				(radius * cos(phi)),
				(radius * sinf(phi) * sinf(theta))
			);

			Vector3 n;
			D3DXVec3Normalize(&n, &p);

			Vector2 uv = Vector2(theta / (Math::PI * 2), phi / Math::PI);

			v.push_back(MeshVertex(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z));
		}
	}
	v.push_back(MeshVertex(0, -radius, 0, 0, 0, 0, -1, 0));


	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex*>(vertices, vertexCount));



	vector<UINT> i;
	for (UINT k = 1; k <= sliceCount; k++)
	{
		i.push_back(0);
		i.push_back(k + 1);
		i.push_back(k);
	}

	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT k = 0; k < stackCount - 2; k++)
	{
		for (UINT j = 0; j < sliceCount; j++)
		{
			i.push_back(baseIndex + k * ringVertexCount + j);
			i.push_back(baseIndex + k * ringVertexCount + j + 1);
			i.push_back(baseIndex + (k + 1) * ringVertexCount + j);

			i.push_back(baseIndex + (k + 1) * ringVertexCount + j);
			i.push_back(baseIndex + k * ringVertexCount + j + 1);
			i.push_back(baseIndex + (k + 1) * ringVertexCount + j + 1);
		}
	}

	UINT southPoleIndex = v.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT k = 0; k < sliceCount; k++)
	{
		i.push_back(southPoleIndex);
		i.push_back(baseIndex + k);
		i.push_back(baseIndex + k + 1);
	}

	indices = new UINT[i.size()];
	indexCount = i.size();

	copy(i.begin(), i.end(), stdext::checked_array_iterator<UINT*>(indices, indexCount));

	int a = 0;
}
