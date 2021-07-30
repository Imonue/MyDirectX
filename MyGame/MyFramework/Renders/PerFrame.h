#pragma once

/*
* Global 쉐이더의 CB_PerFrame를 관리하기 위한 클래스
*/
class PerFrame
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

	void Update();
	void Render();

private:
	struct Desc // 쉐이더안에 있는 CB_PerFrame과 같은 변수를 가지고 있는 구조체
	{
		Matrix View;
		Matrix ViewInverse;
		Matrix Projection;
		Matrix VP;

		Plane Culling[4];
		Plane Clipping;

		float Time;
		float Padding[3];
	} desc;

private:
	Shader* shader;

	ConstantBuffer* buffer;
	ID3DX11EffectConstantBuffer* sBuffer;
};