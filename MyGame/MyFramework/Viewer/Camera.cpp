#include "MyFramework.h"
#include "Camera.h"

Camera::Camera() {
	// 행렬 초기화
	D3DXMatrixIdentity(&matRotation);
	D3DXMatrixIdentity(&matView);

	// 항상 Rotation() 함수를 통해 회전행렬을 먼저 계산
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

	// 위치를 이동해주기 위해서 Move 함수 실행
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

	// 회전 시켜주기 위해서 Move 함수 실행
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
	// 라디안으로 바꾸어주는 수식
	// rotation = vec * Math::PI / 180.0f;
	rotation = vec * 0.01745328f;

	Rotation();
}

void Camera::RotationDegree(Vector3* vec)
{
	// 디그리로 바꾸어주는 함수
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
	D3D가 앞에 붙은 함수들은 행렬을 초기화를 하기 때문에 각각 계산 후 곱해서 회전행렬을 계산
	*/
	Matrix X, Y, Z;
	D3DXMatrixRotationX(&X, rotation.x);
	D3DXMatrixRotationY(&Y, rotation.y);
	D3DXMatrixRotationZ(&Z, rotation.z);

	matRotation = X * Y * Z;

	/*
	일반 좌표를 월드 좌표로 바꾸어주는 함수
	D3DXVec3TransformCoord : 인자값이 위치일때 사용하는 함수
	Vector3를 Vector4로 바꾸어 주는 과정에서 4번째 인자를 1로 주는 것으로
	4x4행렬에서 마지막 열은 이동을 담당하게 되는데 1이 곱해지면 값이 있으므로 이동 사용가능

	D3DXVec3TransformNormal : 인자값이 방향일때 사용되는 함수
	Vector3를 Vector4로 바꾸어 주는 과정에서 4번째 인자를 1로 주는 것으로
	4x4행렬에서 마지막 열은 이동을 담당하게 되는데 0이 곱해지면 값이 모두 0이므로 사용 불가능

	회전행렬에 의해서 카메라가 회전한 만큼 카메라가 가리키고 있는 방향도 변환하는 과정
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
	View 행렬 계산 함수
	D3DXMATRIX *D3DXMatrixLookAtLH(
		D3DXMATRIX *pOut, // 아웃시킬 행렬
		CONST D3DXVECTOR3 *pEye, // 우리가 볼 눈의 위치
		CONST D3DXVECTOR3 *pAt, // 바라볼 방향
		CONST D3DXVECTOR3 *pUp // 카메라의 수직방향을 지정 보통 y축으로 지정
	);

	1. z = At - Eye를 통해서 카메라가 바라보는 방향벡터를 계산후 노말라이즈화
	2. x = up과 위에서 구한 z를 외적(두 방향벡터에 수직인 방향 벡터)를 구한 후 노말라이즈화
	3. y = z, x를 외적(두 방향벡터에 수직인 방향벡터)값을 구한 후 노말라이즈화
	*/
	D3DXMatrixLookAtLH(&matView, &position, &(position + forward), &up);
}