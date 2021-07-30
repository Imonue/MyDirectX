#pragma once

/*
* Global ���̴��� CB_PerFrame�� �����ϱ� ���� Ŭ����
*/
class PerFrame
{
public:
	PerFrame(Shader* shader);
	~PerFrame();

	void Update();
	void Render();

private:
	struct Desc // ���̴��ȿ� �ִ� CB_PerFrame�� ���� ������ ������ �ִ� ����ü
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