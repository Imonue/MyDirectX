#pragma once

class MeshSphere : public Mesh
{
public:
	MeshSphere(Shader* shader, float radius, UINT stackCount = 20, UINT sliceCount = 20);
	~MeshSphere();

private:
	void Create() override;

	// 구를 그릴 반지름
	float radius;

	// 몇개로 분할해서 그릴지 정하는 변수
	UINT stackCount;
	UINT sliceCount;
};