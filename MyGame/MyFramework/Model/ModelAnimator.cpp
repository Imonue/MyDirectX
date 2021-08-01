#include "MyFramework.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(Shader* shader)
	: shader(shader)
{
	model = new Model();
	transform = new Transform(shader);

	frameBuffer = new ConstantBuffer(&tweenDesc, sizeof(TweenDesc));
	sFrameBuffer = shader->AsConstantBuffer("CB_TweenFrame");

	blendBuffer = new ConstantBuffer(&blendDesc, sizeof(BlendDesc));
	sBlendBuffer = shader->AsConstantBuffer("CB_BlendFrame");
}

ModelAnimator::~ModelAnimator()
{
	SafeDelete(model);
	SafeDelete(transform);

	SafeDeleteArray(clipTransforms);
	SafeRelease(texture);
	SafeRelease(srv);

	SafeDelete(frameBuffer);
	SafeDelete(blendBuffer);
}

void ModelAnimator::Update()
{
	if (blendDesc.Mode == 0) // Tween Mode
	{
		UpdateTweenMode();
	}
	else                     // Blend Mode
	{
		UpdateBlendMode();
	}

	if (texture == NULL) // �ؽ�ó�� NULL�̸� ���� �� ���̴��� ����
	{
		for (ModelMesh* mesh : model->Meshes()) // ���̴��� �޽����� �Ҵ�
			mesh->SetShader(shader);

		CreateTexture();
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->Update();
}

void ModelAnimator::UpdateTweenMode()
{
	TweenDesc& desc = tweenDesc;

	//���� �ִϸ��̼�
	{
		ModelClip* clip = model->ClipByIndex(desc.Curr.Clip);
		desc.Curr.RunningTime += Time::Delta();
		float time = 1.0f / clip->FrameRate() / desc.Curr.Speed;
		if (desc.Curr.Time >= 1.0f) // ���� �ð����� �ִϸ��̼� �ݺ�
		{
			desc.Curr.RunningTime = 0;
			desc.Curr.CurrFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount();
			desc.Curr.NextFrame = (desc.Curr.CurrFrame + 1) % clip->FrameCount(); // ������ ���� ������
		}
		desc.Curr.Time = desc.Curr.RunningTime / time;
	}

	if (desc.Next.Clip > -1)
	{
		desc.ChangeTime += Time::Delta();
		desc.TweenTime = desc.ChangeTime / desc.TakeTime;

		if (desc.TweenTime >= 1.0f) // 1���� ũ�ٴ� �ֱ�� �ִϸ��̼� ��ȯ�� �Ϸ�� ����
		{
			desc.Curr = desc.Next;

			desc.Next.Clip = -1;
			desc.Next.CurrFrame = 0;
			desc.Next.NextFrame = 0;
			desc.Next.Time = 0;
			desc.Next.RunningTime = 0.0f;

			desc.ChangeTime = 0.0f;
			desc.TweenTime = 0.0f;
		}
		else // ��ȯ�� �Ϸᰡ ���� ���� ����
		{
			ModelClip* clip = model->ClipByIndex(desc.Next.Clip);
			desc.Next.RunningTime += Time::Delta();
			float time = 1.0f / clip->FrameRate() / desc.Next.Speed;
			if (desc.Next.Time >= 1.0f) // ���� �ð����� �ִϸ��̼� �ݺ�
			{
				desc.Next.RunningTime = 0;
				desc.Next.CurrFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount();
				desc.Next.NextFrame = (desc.Next.CurrFrame + 1) % clip->FrameCount(); // ������ ���� ������
			}
			desc.Next.Time = desc.Next.RunningTime / time;
		}
	}
}

void ModelAnimator::UpdateBlendMode()
{
	BlendDesc& desc = blendDesc;

	for (UINT i = 0; i < 3; i++)
	{
		ModelClip* clip = model->ClipByIndex(desc.Clip[i].Clip);
		desc.Clip[i].RunningTime += Time::Delta();
		float time = 1.0f / clip->FrameRate() / desc.Clip[i].Speed;
		if (desc.Clip[i].Time >= 1.0f) // ���� �ð����� �ִϸ��̼� �ݺ�
		{
			desc.Clip[i].RunningTime = 0;
			desc.Clip[i].CurrFrame = (desc.Clip[i].CurrFrame + 1) % clip->FrameCount();
			desc.Clip[i].NextFrame = (desc.Clip[i].CurrFrame + 1) % clip->FrameCount(); // ������ ���� ������
		}
		desc.Clip[i].Time = desc.Clip[i].RunningTime / time;
	}
}

void ModelAnimator::Render()
{
	frameBuffer->Render();
	sFrameBuffer->SetConstantBuffer(frameBuffer->Buffer());

	blendBuffer->Render();
	sBlendBuffer->SetConstantBuffer(blendBuffer->Buffer());

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

void ModelAnimator::PlayTweenMode(UINT clip, float speed, float takeTime)
{
	blendDesc.Mode = 0;

	tweenDesc.TakeTime = takeTime;
	tweenDesc.Next.Clip = clip;
	tweenDesc.Next.Speed = speed;
}

void ModelAnimator::PlayBlendMode(UINT clip, UINT clip1, UINT clip2)
{
	blendDesc.Mode = 1;

	blendDesc.Clip[0].Clip = clip;
	blendDesc.Clip[1].Clip = clip1;
	blendDesc.Clip[2].Clip = clip2;
}

void ModelAnimator::SetBlendAlpha(float alpha)
{
	alpha = Math::Clamp(alpha, 0.0f, 2.0f); // ���� ���� 0.0f ~ 2.0f������ ����
	blendDesc.Alpha = alpha;
}

void ModelAnimator::CreateTexture()
{
	//Matrix matrix[MAX_MODEL_KEYFRAMES][MAX_MODEL_TRANSFORMS];

	clipTransforms = new ClipTransform[model->ClipCount()]; // Ŭ���� ������ŭ ���� �Ҵ�
	for (UINT i = 0; i < model->ClipCount(); i++)
		CreateClipTransform(i); // Ŭ������ ��� ���� �����Ǿ� �ִ� ����ü ���� �� �Ҵ�


	//Create Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4; // ����� �� ��Ĵ� �� 4����Ʈ�ε� DX10���� �ؽ�ó�� �ȼ� �ϳ��� 16����Ʈ�̹Ƿ� * 4
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = model->ClipCount(); // ���� ����
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; //16Byte * 4 = 64Byte
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // ���̴� ���ҽ� �並 ���� ���̴����� ����
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		UINT pageSize = MAX_MODEL_TRANSFORMS * 4 * 16 * MAX_MODEL_KEYFRAMES;
		//void* p = malloc(pageSize * model->ClipCount());
		void* p = VirtualAlloc(NULL, pageSize * model->ClipCount(), MEM_RESERVE, PAGE_READWRITE); // malloc�� �����Լ������� �����ϹǷ� ���������� 2MB�� �Ѿ �� ���� ������ ����޸��Ҵ� �Լ��� virtualAlloc�� ���

		//MEMORY_BASIC_INFORMATION, VirtualQuery�� ���� VirtualAlloc���� ������ ����� �� �� �ִ�

		for (UINT c = 0; c < model->ClipCount(); c++)
		{
			UINT start = c * pageSize; // Ŭ���� ���� �ּ�

			for (UINT k = 0; k < MAX_MODEL_KEYFRAMES; k++) // ���� ���� ũ��
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
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // �ؽ�ó�� ��� �������� �ٷ� �������� ����
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // ���ҽ��� ����
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = model->ClipCount();

		Check(D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &srv)); // ���̴� ���ҽ� ����
	}

	for (ModelMesh* mesh : model->Meshes())
		mesh->TransformsSRV(srv);
}

void ModelAnimator::CreateClipTransform(UINT index)
{
	Matrix* bones = new Matrix[MAX_MODEL_TRANSFORMS]; // �θ� ����� ������ ����

	ModelClip* clip = model->ClipByIndex(index); // �𵨿��� index�� ������ Ŭ���� �ҷ��ͼ� �Ҵ�
	for (UINT f = 0; f < clip->FrameCount(); f++) // Ű������(��)���� �ݺ��ϸ鼭 �Ҵ�
	{
		for (UINT b = 0; b < model->BoneCount(); b++) // ������(��) ������ŭ �ݺ��ؼ� �Ҵ�
		{
			ModelBone* bone = model->BoneByIndex(b); // �� ������ �ε����μ� ����


			Matrix parent; // �θ���踦 ����� ���� ���
			Matrix invGlobal = bone->Transform();
			D3DXMatrixInverse(&invGlobal, NULL, &invGlobal); // �����

			int parentIndex = bone->ParentIndex(); // �θ��� �ε����� ��Ƴ��� ����
			if (parentIndex < 0) // 0���� ������ root�̹Ƿ� �Ұ�
				D3DXMatrixIdentity(&parent);
			else
				parent = bones[parentIndex];


			Matrix animation;
			ModelKeyframe* frame = clip->Keyframe(bone->Name()); // Ŭ�����κ��� ���� �̸��� ���ؼ� �ش��ϴ� ���� Ű �������� �ҷ��ͼ� �Ҵ�

			if (frame != NULL) // Ű �������� ���� ���
			{
				ModelKeyframeData& data = frame->Transforms[f];

				Matrix S, R, T;
				D3DXMatrixScaling(&S, data.Scale.x, data.Scale.y, data.Scale.z);
				D3DXMatrixRotationQuaternion(&R, &data.Rotation); // ȸ���� ���ʹϾ����� ����Ǿ� �����Ƿ� �ش��ϴ� �Լ��� ���
				D3DXMatrixTranslation(&T, data.Translation.x, data.Translation.y, data.Translation.z);

				animation = S * R * T;
			}
			else
			{
				/*
				* �ִϸ��̼��� Ű �����Ͱ� ���� ��� �ڽ��� �� ��� ������ �����س��� �Ϳ� Identuty�� �����
				* ���� ��� �� ����� �θ�� ���ؼ� �ڽ��� �������� �׳� �θ� ������� ����
				*/
				D3DXMatrixIdentity(&animation);
			}

			/*
			* ����� �θ� ����� ���� ���� �迭�ε� �θ���� �����ͼ� �ִϸ��̼� ���� ����� ���ؼ� ��� �ٽ� �ִµ�
			* �̰��� �θ� �ڽ� ���Ը� �δ� �Ͱ� ����
			* �ִϸ��̼� ����� �ش� �����ӿ��� �ش� ���� �󸶸�ŭ �̵��Ұ������� �����ϴµ� �θ� ���ϴ� ���� �θ� ������ ��������
			* ��������� �߰��� �������� �ϱ� ����
			* Converter::ReadBoneData���� bone->Transform(Global) = bone->Transform(Relative) * matParent(Global) �̹Ƿ�
			* ���� parent�� Global�̹Ƿ� animation(Relative)�� ���ϸ� ���������� bones[b](Global)�� �Ǿ� �Ҵ�
			* bone�� Ʈ�������� ���� Global �����̹Ƿ� ������� �Ͽ��� Relative�� ���·� ���� �� inGlobal(Relative) * bones[b](Global)�� ����
			* ���������� �ִϸ��̼ǰ� �θ��� ���� �ڽ��� Ʈ�������� ���
			*/
			bones[b] = animation * parent;
			clipTransforms[index].Transform[f][b] = invGlobal * bones[b];
		}//for(b)
	}//for(f)
}
