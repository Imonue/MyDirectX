#pragma once

class Camera {
public:
	Camera();
	virtual ~Camera();
	virtual void Update() = 0;

public:
	// 위치를 설정하는 함수와 반환하는 함수
	void Position(float x, float y, float z);
	void Position(Vector3& vector);
	void Position(Vector3* vector);

	// 회전각(라디안)을 설정하는 함수와 반환하는 함수
	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vector);
	void Rotation(Vector3* vector);

	// 회전각(디그리)을 설정하는 함수와 반환하는 함수
	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vector);
	void RotationDegree(Vector3* vector);

	// 행렬 반환 함수
	void GetMatrix(Matrix* matrix);

	// 방향을 반환하는 함수
	Vector3 Forward() { return forward; }
	Vector3 Up() { return up; }
	Vector3 Right() { return right; }

protected:
	// 자식에서 사용하는 이동이나 회전 관련 함수
	virtual void Rotation();
	virtual void Move();

protected:
	// 뷰 행렬을 계산해주는 함수
	void View();

protected:
	// 뷰 행렬
	Matrix matView;

private:
	// 카메라 위치 함수
	Vector3 position = Vector3(0, 0, 0);

	// 어떤 방향으로 돌것인지 정하는 벡터, 이용해서 회전행렬 제작
	Vector3 rotation = Vector3(0, 0, 0);

	// 전방방향
	Vector3 forward = Vector3(0, 0, 1);

	// 위 방향
	Vector3 up = Vector3(0, 1, 0);

	// 오른쪽 방향
	Vector3 right = Vector3(1, 0, 0);

	// 회전 행렬
	Matrix matRotation;
};