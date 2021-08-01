#pragma once

class ModelAnimator
{
public:
	ModelAnimator(Shader* shader);
	~ModelAnimator();

	void Update();

private:
	void UpdateTweenMode();
	void UpdateBlendMode();

public:
	void Render();

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

	Transform* GetTransform() { return transform; }
	Model* GetModel() { return model; }

	void Pass(UINT pass);

	void PlayTweenMode(UINT clip, float speed = 1.0f, float takeTime = 1.0f); // takeTime - �� ���۰� ���� ������ ��ȯ�ɶ������� �ҿ�ð�
	void PlayBlendMode(UINT clip, UINT clip1, UINT clip2);
	void SetBlendAlpha(float alpha);

private:
	// ������ �Ҵ��� �� �ִ� �Լ�
	void CreateTexture();
	void CreateClipTransform(UINT index);

private:
	/*
	* Ű�����Ӱ� �������� Ŭ���� �Ҵ��� ����ü
	* Ű������(��) - 1�� ���� 1�� Ű������, 2�� ���� 2�� Ű������
	* ������(��) - 1�� ���� �ش��ϴ� ��(Ű��������) 1�� ���� �ش��ϴ� Ű ������
	* Ŭ��(��) - ���� ũ��� �����Ǿ� �ְ� ���� ��� ���� ��� ����ü �迭�� �����Ҵ����μ� ����
	*/
	struct ClipTransform
	{
		Matrix** Transform; // ����� ���� 2���� ���

		ClipTransform()
		{
			Transform = new Matrix * [MAX_MODEL_KEYFRAMES]; // ���� �ִ� Ű������ ����

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS]; // �ึ�� �� ����(��)�� �ʱ�ȭ
		}

		~ClipTransform()
		{
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};
	ClipTransform* clipTransforms = NULL; // �������� Ŭ���鸶�� �����Ҵ����μ� ��,�� ������ ��� ����ü�� �־�� �ϹǷ� ������ �迭�� ����

	ID3D11Texture2D* texture = NULL; // �ؽ�ó�� �ѱ�� ���ؼ� ����
	ID3D11ShaderResourceView* srv = NULL; // �ؽ�ó�� ���̴��� �Ѱ��ֱ� ���ؼ� ������ ���ҽ��� ����

private:
	struct KeyframeDesc // ���̴��� �ѱ������ ����ü
	{
		int Clip = 0; // �����÷����Ϸ����ϴ� �ִϸ��̼� ��ȣ

		UINT CurrFrame = 0; // ���� ������ ��ȣ
		UINT NextFrame = 0; // ���� ������ ��ȣ

		float Time = 0.0f; // ���� �÷������� �ð�
		float RunningTime = 0.0f; // �ִϸ��̼� �ð�

		float Speed = 1.0f; // �ִϸ��̼� �ӵ�

		Vector2 Padding;
	}; //keyframeDesc;

	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

	// ���� ���۰� ���������� ������ �� �ִ� ����ü
	struct TweenDesc
	{
		float TakeTime = 1.0f; // �ִϸ��̼��� ��ȭ�� �ð�
		float TweenTime = 0.0f;// ���ذ��� �ð��� ���
		float ChangeTime = 0.0f;
		float Padding;

		KeyframeDesc Curr; // ���� ����
		KeyframeDesc Next; // ���� ����

		TweenDesc()
		{
			Curr.Clip = 0;
			Next.Clip = -1;
		}
	} tweenDesc;

private:
	// �ִϸ��̼� Blend�� ���� ����ü
	struct BlendDesc
	{
		UINT Mode = 0; // Mode�� 0���� ũ�� BlendMode�� �۾��� �� ���̰� 0�̶�� TweenMode�� ����
		float Alpha = 0; // �ִϸ��̼��� ���� ���� �𸮾������� ��� ��
		Vector2 Padding;

		KeyframeDesc Clip[3]; // ���� 3���� ������Ų�ٴ� ����
	} blendDesc;

	ConstantBuffer* blendBuffer;
	ID3DX11EffectConstantBuffer* sBlendBuffer;

private:
	Shader* shader;
	Model* model;
	Transform* transform;
};