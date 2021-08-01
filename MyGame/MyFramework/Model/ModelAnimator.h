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

	void PlayTweenMode(UINT clip, float speed = 1.0f, float takeTime = 1.0f); // takeTime - 한 동작과 다음 동작이 전환될때까지에 소요시간
	void PlayBlendMode(UINT clip, UINT clip1, UINT clip2);
	void SetBlendAlpha(float alpha);

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
	}; //keyframeDesc;

	ConstantBuffer* frameBuffer;
	ID3DX11EffectConstantBuffer* sFrameBuffer;

	// 현재 동작과 다음동작을 관리할 수 있는 구조체
	struct TweenDesc
	{
		float TakeTime = 1.0f; // 애니메이션이 변화할 시간
		float TweenTime = 0.0f;// 변해가는 시간을 기록
		float ChangeTime = 0.0f;
		float Padding;

		KeyframeDesc Curr; // 현재 동작
		KeyframeDesc Next; // 다음 동작

		TweenDesc()
		{
			Curr.Clip = 0;
			Next.Clip = -1;
		}
	} tweenDesc;

private:
	// 애니메이션 Blend를 위한 구조체
	struct BlendDesc
	{
		UINT Mode = 0; // Mode가 0보다 크면 BlendMode로 작업이 될 것이고 0이라면 TweenMode로 진행
		float Alpha = 0; // 애니메이션을 섞을 정도 언리얼엔진에서 녹색 점
		Vector2 Padding;

		KeyframeDesc Clip[3]; // 동작 3개를 고정시킨다는 가정
	} blendDesc;

	ConstantBuffer* blendBuffer;
	ID3DX11EffectConstantBuffer* sBlendBuffer;

private:
	Shader* shader;
	Model* model;
	Transform* transform;
};