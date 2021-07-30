#pragma once

class Camera {
public:
	Camera();
	virtual ~Camera();
	virtual void Update() = 0;

public:
	// ��ġ�� �����ϴ� �Լ��� ��ȯ�ϴ� �Լ�
	void Position(float x, float y, float z);
	void Position(Vector3& vector);
	void Position(Vector3* vector);

	// ȸ����(����)�� �����ϴ� �Լ��� ��ȯ�ϴ� �Լ�
	void Rotation(float x, float y, float z);
	void Rotation(Vector3& vector);
	void Rotation(Vector3* vector);

	// ȸ����(��׸�)�� �����ϴ� �Լ��� ��ȯ�ϴ� �Լ�
	void RotationDegree(float x, float y, float z);
	void RotationDegree(Vector3& vector);
	void RotationDegree(Vector3* vector);

	// ��� ��ȯ �Լ�
	void GetMatrix(Matrix* matrix);

	// ������ ��ȯ�ϴ� �Լ�
	Vector3 Forward() { return forward; }
	Vector3 Up() { return up; }
	Vector3 Right() { return right; }

protected:
	// �ڽĿ��� ����ϴ� �̵��̳� ȸ�� ���� �Լ�
	virtual void Rotation();
	virtual void Move();

protected:
	// �� ����� ������ִ� �Լ�
	void View();

protected:
	// �� ���
	Matrix matView;

private:
	// ī�޶� ��ġ �Լ�
	Vector3 position = Vector3(0, 0, 0);

	// � �������� �������� ���ϴ� ����, �̿��ؼ� ȸ����� ����
	Vector3 rotation = Vector3(0, 0, 0);

	// �������
	Vector3 forward = Vector3(0, 0, 1);

	// �� ����
	Vector3 up = Vector3(0, 1, 0);

	// ������ ����
	Vector3 right = Vector3(1, 0, 0);

	// ȸ�� ���
	Matrix matRotation;
};