#include "MyFramework.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(Shader* shader)
	: shader(shader)
{
	model = new Model();
	transform = new Transform(shader);

	frameBuffer = new ConstantBuffer(&keyframeDesc, sizeof(KeyframeDesc));
	sFrameBuffer = shader->AsConstantBuffer("CB_AnimationFrame");
}

ModelAnimator::~ModelAnimator()
{
	SafeDelete(model);
	SafeDelete(transform);

	SafeDeleteArray(clipTransforms);
	SafeRelease(texture);
	SafeRelease(srv);

	SafeDelete(frameBuffer);
}

void ModelAnimator::Update()
{
	ImGui::InputInt("Clip", &keyframeDesc.Clip);
	keyframeDesc.Clip %= model->ClipCount();

	ImGui::InputInt("CurrFrame", (int*)&keyframeDesc.CurrFrame);
	keyframeDesc.CurrFrame = model->ClipByIndex(keyframeDesc.Clip)->FrameCount();


	if (texture == NULL) // 텍스처가 NULL이면 생성 후 쉐이더로 전달
	{
		for (ModelMesh* mesh : model->Meshes()) // 쉐이더를 메쉬마다 할당
			mesh->SetShader(shader);

		CreateTexture();
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->Update();
}

void ModelAnimator::Render()
{
	frameBuffer->Render();
	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

	for (ModelMesh* mesh : model->Meshes())
	{
		mesh->SetTransform(transform);
		mesh->Render();
	}
}

void ModelAnimator::ReadMesh(wstring file)
{
	model->ReadMesh(file);
}

void ModelAnimator::ReadMaterial(wstring file)
{
	model->ReadMaterial(file);
}

void ModelAnimator::ReadClip(wstring file)
{
	model->ReadClip(file);
}

void ModelAnimator::Pass(UINT pass)
{
	for (ModelMesh* mesh : model->Meshes())
		mesh->Pass(pass);
}

void ModelAnimator::CreateTexture()
{
	//Matrix matrix[MAX_MODEL_KEYFRAMES][MAX_MODEL_TRANSFORMS];

	clipTransforms = new ClipTransform[model->ClipCount()]; // 클립의 개수만큼 동적 할당
	for (UINT i = 0; i < model->ClipCount(); i++)
		CreateClipTransform(i); // 클립별로 행과 열로 구성되어 있는 구조체 생성 후 할당


	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4; // 행렬은 한 행렬당 총 4바이트인데 DX10부터 텍스처의 픽셀 하나당 16바이트이므로 * 4
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = model->ClipCount(); // 면의 개수
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; //16Byte * 4 = 64Byte
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 쉐이더 리소스 뷰를 통해 쉐이더에게 전달
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		//void* p = malloc(pageSize * model->ClipCount());
		void* p = VirtualAlloc(NULL, pageSize * model->ClipCount(), MEM_RESERVE, PAGE_READWRITE); // malloc은 지역함수내에서 실행하므로 스택프레임 2MB를 넘어설 수 없기 때문에 가상메모리할당 함수인 virtualAlloc를 사용

		//MEMORY_BASIC_INFORMATION, VirtualQuery를 통해 VirtualAlloc으로 예약한 사이즈를 알 수 있다

		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			UINT start = c * pageSize; // 클립의 시작 주소

			for (UINT k = 0; k < MAX_MODEL_KEYFRAMES; k++) // 면의 세로 크기
			{
				void* temp = (BYTE*)p + MAX_MODEL_TRANSFORMS * k * sizeof(Matrix) + start;

				VirtualAlloc(temp, MAX_MODEL_TRANSFORMS * sizeof(Matrix), MEM_COMMIT, PAGE_READWRITE);
				memcpy(temp, clipTransforms[c].Transform[k], MAX_MODEL_TRANSFORMS * sizeof(Matrix));
			}
		}//for(c)


		D3D11_SUBRESOURCE_DATA* subResources = new D3D11_SUBRESOURCE_DATA[model->ClipCount()];
		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			void* temp = (BYTE*)p + c * pageSize;

			subResources[c].pSysMem = temp;
			subResources[c].SysMemPitch = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
			subResources[c].SysMemSlicePitch = pageSize;
		}
		Check(D3D::GetDevice()->CreateTexture2D(&desc, subResources, &texture));


		SafeDeleteArray(subResources);
		VirtualFree(p, 0, MEM_RELEASE);
	}


	//Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 텍스처를 어떠한 ㅍ맷으로 다룰 것인지를 지정
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // 리소스의 형식
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = model->ClipCount();

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &srv)); // 쉐이더 리소스 생성
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->TransformsSRV(srv);
}

void ModelAnimator::CreateClipTransform(UINT index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS]; // 부모 행렬을 저장할 변수

	ModelClip* clip = model->ClipByIndex(index); // 모델에서 index로 지정한 클립을 불러와서 할당
	for (UINT f = 0; f < clip->FrameCount(); f++) // 키프레임(행)부터 반복하면서 할당
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // 본정보(열) 개수만큼 반복해서 할당
		{
			ModelBone* bone = model->BoneByIndex(b); // 본 정보를 인덱스로서 참조


			Matrix parent; // 부모관계를 씌우기 위한 행렬
			Matrix invGlobal = bone->Transform();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal); // 역행렬

			int parentIndex = bone->ParentIndex(); // 부모의 인덱스를 담아놓는 변수
			if (parentIndex < 0) // 0보다 작으면 root이므로 불가
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];


			Matrix animation;
			ModelKeyframe* frame = clip->Keyframe(bone->Name()); // 클립으로부터 본의 이름을 통해서 해당하는 본의 키 프레임을 불러와서 할당

			if (frame != NULL) // 키 프레임이 있을 경우
			{
				ModelKeyframeData& data = frame->Transforms[f];

				Matrix S, R, T;
				D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
				D3DXMatrixRotationQuaternion(&R, &data.Rotation); // 회저은 쿼터니안으로 저장되어 있으므로 해당하는 함수를 사용
				D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

				animation = S * R * T;
			}
			else
			{
				/*
				* 애니메이션의 키 데이터가 없을 경우 자신의 본 노드 정보를 저장해놓은 것에 Identuty된 행렬을
				* 곱할 경우 이 행렬을 부모랑 곱해서 자신의 움직임은 그냥 부모를 따르드록 구현
				*/
				D3DXMatrixIdentity(&animation);
			}

			/*
			* 본즈는 부모 행렬을 담은 동적 배열인데 부모것을 가져와서 애니메이션 만든 행렬을 곱해서 본즈에 다시 넣는데
			* 이것은 부모 자식 관게를 맺는 것과 동일
			* 애니메이션 행렬은 해당 프레임에서 해당 본이 얼마만큼 이동할것인지를 결정하는데 부모를 곱하는 것은 부모가 움직인 지점에서
			* 상대적으로 추가로 움직여야 하기 때문
			* Converter::ReadBoneData에서 bone->Transform(Global) = bone->Transform(Relative) * matParent(Global) 이므로
			* 현재 parent는 Global이므로 animation(Relative)를 곱하면 최종적으로 bones[b](Global)로 되어 할당
			* bone의 트랜스폼은 현재 Global 상태이므로 역행렬을 하여서 Relative한 상태로 만든 후 inGlobal(Relative) * bones[b](Global)를 통해
			* 최종적으로 애니메이션과 부모의 따른 자신의 트랜스폼이 계산
			*/
			bones[b] = animation * parent;
			clipTransforms[index].Transform[f][b] = invGlobal * bones[b];
		}//for(b)
	}//for(f)
}
