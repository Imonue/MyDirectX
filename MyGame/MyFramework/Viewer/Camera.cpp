#include "MyFramework.h"
#include "Camera.h"

Camera::Camera() {
	// ��� �ʱ�ȭ
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	// �׻� Rotation() �Լ��� ���� ȸ������� ���� ���
	Rotation();
	Move();
}

Camera::~Camera() {

}


void Camera::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Camera::Position(Vector3& vec)
{
	position = vec;

	// ��ġ�� �̵����ֱ� ���ؼ� Move �Լ� ����
	Move();
}

void Camera::Position(Vector3* vec)
{
	*vec = position;
}

void Camera::Rotation(float x, float y, float z)
{
	Rotation(Vector3(x, y, z));
}

void Camera::Rotation(Vector3& vec)
{
	rotation = vec;

	// ȸ�� �����ֱ� ���ؼ� Move �Լ� ����
	Rotation();
}

void Camera::Rotation(Vector3* vec)
{
	*vec = rotation;
}

void Camera::RotationDegree(float x, float y, float z)
{
	RotationDegree(Vector3(x, y, z));
}

void Camera::RotationDegree(Vector3& vec)
{
	// �������� �ٲپ��ִ� ����
	// rotation = vec * Math::PI / 180.0f;
	rotation = vec * 0.01745328f;

	Rotation();
}

void Camera::RotationDegree(Vector3* vec)
{
	// ��׸��� �ٲپ��ִ� �Լ�
	//*vec = rotation * 180.0f / Math::PI;
	*vec = rotation * 57.29577957f;
}

void Camera::GetMatrix(Matrix* matrix)
{
	//*matrix = matView;
	memcpy(matrix, &matView, sizeof(Matrix));
}

void Camera::Rotation()
{
	/*
	D3D�� �տ� ���� �Լ����� ����� �ʱ�ȭ�� �ϱ� ������ ���� ��� �� ���ؼ� ȸ������� ���
	*/
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);
	D3DXMatrixRotationY(&Y, rotation.y);
	D3DXMatrixRotationZ(&Z, rotation.z);

	matRotation = X * Y * Z;

	/*
	�Ϲ� ��ǥ�� ���� ��ǥ�� �ٲپ��ִ� �Լ�
	D3DXVec3TransformCoord : ���ڰ��� ��ġ�϶� ����ϴ� �Լ�
	Vector3�� Vector4�� �ٲپ� �ִ� �������� 4��° ���ڸ� 1�� �ִ� ������
	4x4��Ŀ��� ������ ���� �̵��� ����ϰ� �Ǵµ� 1�� �������� ���� �����Ƿ� �̵� ��밡��

	D3DXVec3TransformNormal : ���ڰ��� �����϶� ���Ǵ� �Լ�
	Vector3�� Vector4�� �ٲپ� �ִ� �������� 4��° ���ڸ� 1�� �ִ� ������
	4x4��Ŀ��� ������ ���� �̵��� ����ϰ� �Ǵµ� 0�� �������� ���� ��� 0�̹Ƿ� ��� �Ұ���

	ȸ����Ŀ� ���ؼ� ī�޶� ȸ���� ��ŭ ī�޶� ����Ű�� �ִ� ���⵵ ��ȯ�ϴ� ����
	*/
	D3DXVec3TransformNormal(&forward, &Vector3(0, 0, 1), &matRotation);
	D3DXVec3TransformNormal(&up, &Vector3(0, 1, 0), &matRotation);
	D3DXVec3TransformNormal(&right, &Vector3(1, 0, 0), &matRotation);
}

void Camera::Move()
{
	View();
}

void Camera::View()
{
	/*
	View ��� ��� �Լ�
	D3DXMATRIX *D3DXMatrixLookAtLH(
		D3DXMATRIX *pOut, // �ƿ���ų ���
		CONST D3DXVECTOR3 *pEye, // �츮�� �� ���� ��ġ
		CONST D3DXVECTOR3 *pAt, // �ٶ� ����
		CONST D3DXVECTOR3 *pUp // ī�޶��� ���������� ���� ���� y������ ����
	);

	1. z = At - Eye�� ���ؼ� ī�޶� �ٶ󺸴� ���⺤�͸� ����� �븻������ȭ
	2. x = up�� ������ ���� z�� ����(�� ���⺤�Ϳ� ������ ���� ����)�� ���� �� �븻������ȭ
	3. y = z, x�� ����(�� ���⺤�Ϳ� ������ ���⺤��)���� ���� �� �븻������ȭ
	*/
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);
}