#pragma once

class ModelAnimator
{
public:
	ModelAnimator(Shader* shader);
	~ModelAnimator();

	void Update();
	void Render();

public:
	void ReadMesh(wstring file);
	void ReadMaterial(wstring file);
	void ReadClip(wstring file);

	Transform* GetTransform() { return transform; }
	Model* GetModel() { return model; }

	void Pass(UINT pass);

private:
	// 실제로 할당할 수 있는 함수
	void CreateTexture();
	void CreateClipTransform(UINT index);

private:
	/*
	* 키프레임과 본정보와 클립을 할당할 구조체
	* 키프레임(행) - 1번 행은 1번 키프레임, 2번 행은 2번 키프레임
	* 본정보(열) - 1번 열에 해당하는 행(키프레임은) 1번 본에 해당하는 키 프레임
	* 클립(면) - 일정 크기로 고정되어 있고 면은 행과 열이 담긴 구조체 배열에 동적할당으로서 구성
	*/
	struct ClipTransform
	{
		Matrix** Transform; // 멤버를 넣을 2차원 행렬

		ClipTransform()
		{
			Transform = new Matrix * [MAX_MODEL_KEYFRAMES]; // 행은 최대 키프레임 개수

			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				Transform[i] = new Matrix[MAX_MODEL_TRANSFORMS]; // 행마다 본 정보(열)을 초기화
		}

		~ClipTransform()
		{
			for (UINT i = 0; i < MAX_MODEL_KEYFRAMES; i++)
				SafeDeleteArray(Transform[i]);

			SafeDeleteArray(Transform);
		}
	};
	ClipTransform* clipTransforms = NULL; // 여러개의 클립들마다 동적할당으로서 행,열 정보가 담긴 구조체가 있어야 하므로 포인터 배열로 구현

	ID3D11Texture2D* texture = NULL; // 텍스처로 넘기기 위해서 변수
	ID3D11ShaderResourceView* srv = NULL; // 텍스처를 쉐이더로 넘겨주기 위해서 생성한 리소스뷰 변수

private:
	struct KeyframeDesc // 쉐이더에 넘기기위한 구조체
	{
		int Clip = 0; // 현재플레이하려고하는 애니메이션 번호

		UINT CurrFrame = 0; // 현재 프레임 번호
		UINT NextFrame = 0; // 다음 프레임 번호

		float Time = 0.0f; // 현재 플레이중인 시간
		float RunningTime = 0.0f; // 애니메이션 시간

		float Speed = 1.0f; // 애니메이션 속도

		Vector2 Padding;
	} keyframeDesc;

	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

private:
	Shader* shader;
	Model* model;
	Transform* transform;
};