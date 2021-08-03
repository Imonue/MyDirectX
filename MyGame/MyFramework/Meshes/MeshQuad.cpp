#include "MyFramework.h"
#include "MeshQuad.h"

MeshQuad::MeshQuad()
{

}

MeshQuad::~MeshQuad()
{

}

void MeshQuad::Create()
{
	float w = 0.5f;
	float h = 0.5f;

	vector<MeshVertex> v;
	v.push_back(MeshVertex(-w, -h, 0, 0, 1, 0, 0, -1));
	v.push_back(MeshVertex(-w, +h, 0, 0, 0, 0, 0, -1));
	v.push_back(MeshVertex(+w, -h, 0, 1, 1, 0, 0, -1));
	v.push_back(MeshVertex(+w, +h, 0, 1, 0, 0, 0, -1));


	vertices = new MeshVertex[v.size()];
	vertexCount = v.size();

	/*
	* stdext - std의 확장형으로서 복사받은 배열의 시작 주소와 마지막 주소를 받아서 안정 
	*/
	copy(v.begin(), v.end(), stdext::checked_array_iterator<MeshVertex*>(vertices, vertexCount));


	indexCount = 6;
	indices = new UINT[indexCount]{ 0, 1, 2, 2, 1, 3 };
}
