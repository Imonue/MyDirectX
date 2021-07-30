#include "stdafx.h"
#include "Converter.h"
#include "Types.h"
#include "Utilities/BinaryFile.h"
#include "Utilities/Xml.h"

Converter::Converter()
{
	importer = new Assimp::Importer();
}

Converter::~Converter()
{
	SafeDelete(importer);
}

void Converter::ReadFile(wstring file)
{
	this->file = L"../../_Assets/" + file; // 파일 저장 경로

	scene = importer->ReadFile
	(
		String::ToString(this->file),
		aiProcess_ConvertToLeftHanded // 기본적인 모든 3D 모델들은 오른손좌표계를 사용하기 때문에 전환하는 의미
		| aiProcess_Triangulate       // 모델을 디자인하는 방법으로 삼각형을 기준으로 변환해서 데이터를 전달해달라는 의미
		| aiProcess_GenUVCoords       // UV좌표를 변환해달라는 의미
		| aiProcess_GenNormals		  // 삼각형으로 노말도 다시 변환해달라는 의미
		| aiProcess_CalcTangentSpace
	);
	assert(scene != NULL); // scene이 불러올 수 없는 파일이거나 없는 포맷일 경우 null이 출력
}

void Converter::ExportMesh(wstring savePath)
{
	savePath = L"../../_Models/" + savePath + L".mesh"; // 파일 저장 경로

	ReadBoneData(scene->mRootNode, -1, -1);
	ReadSkinData();

	//Write CSV File
	{
		FILE* file;
		fopen_s(&file, "../Vertices.csv", "w");

		for (asBone* bone : bones)
		{
			string name = bone->Name;
			fprintf(file, "%d,%s\n", bone->Index, bone->Name.c_str());
		}


		fprintf(file, "\n");
		for (asMesh* mesh : meshes)
		{
			string name = mesh->Name;
			printf("%s\n", name.c_str());

			for (UINT i = 0; i < mesh->Vertices.size(); i++)
			{
				Vector3 p = mesh->Vertices[i].Position;
				Vector4 indices = mesh->Vertices[i].BlendIndices;
				Vector4 weights = mesh->Vertices[i].BlendWeights;

				fprintf(file, "%f,%f,%f,", p.x, p.y, p.z);
				fprintf(file, "%f,%f,%f,%f,", indices.x, indices.y, indices.z, indices.w);
				fprintf(file, "%f,%f,%f,%f\n", weights.x, weights.y, weights.z, weights.w);
			}
		}

		fclose(file);
	}

	WriteMeshData(savePath);
}

void Converter::ReadBoneData(aiNode* node, int index, int parent)
{
	// TODO: 본 정보 읽기
	// TODO: 메시 정보 읽기

	asBone* bone = new asBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str();


	Matrix transform(node->mTransformation[0]);
	D3DXMatrixTranspose(&bone->Transform, &transform); // 열우선을 행우선으로 전치 해주는 함수

	Matrix matParent;
	if (parent < 0) // parent가 루트일 경우
		D3DXMatrixIdentity(&matParent); // 
	else
		matParent = bones[parent]->Transform;

	bone->Transform = bone->Transform * matParent; // 부모만큼 움직이기 위해서 곱
	bones.push_back(bone); // 본이 추가가 되면 인덱스가 증가되서 아래 반복문에서 자신의 인덱스를 자식들이 지정 가능


	ReadMeshData(node, index);

	for (UINT i = 0; i < node->mNumChildren; i++)
		ReadBoneData(node->mChildren[i], bones.size(), index); // 자신의 인덱스가 자식의 부모로 지정
	int a = 10;
}

void Converter::ReadMeshData(aiNode* node, int bone)
{
	if (node->mNumMeshes < 1) return;

	asMesh* mesh = new asMesh();
	mesh->Name = node->mName.C_Str();
	mesh->BoneIndex = bone;

	for (UINT i = 0; i < node->mNumMeshes; i++) // scene에 있는 모든 mesh를 불러올 예정
	{
		UINT index = node->mMeshes[i]; // mesh의 정보가 아닌 scene이 가지고 있는 mesh배열에서 참조하고 있는 인덱스를 반환
		aiMesh* srcMesh = scene->mMeshes[index];

		aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->MaterialName = material->GetName().C_Str();


		UINT startVertex = mesh->Vertices.size(); // 정점의 시작 번호를 가지고 있어야 메시가 여러 개가 되어도 정점을 계속 이어서 저장 가능하도록 이전에 쌓아놓은 크기를 할당
		for (UINT v = 0; v < srcMesh->mNumVertices; v++) // 정점의 개수만큼 반복
		{
			Model::ModelVertex vertex; // 정점 버텍스 객체
			memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3)); // 메시가 존재한다면 Position은 항상 존재

			if (srcMesh->HasTextureCoords(0)) // 텍스처 좌표를 가지고 있는지 판단하는 조건문
				memcpy(&vertex.Uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2)); // 레이어 떄문에 2차원 배열이므로 [0]을 반드시 입력

			if (srcMesh->HasNormals()) // 노말 좌표를 가지고 있는지를 판단
				memcpy(&vertex.Normal, &srcMesh->mNormals[v], sizeof(Vector3));

			mesh->Vertices.push_back(vertex);
		}

		for (UINT f = 0; f < srcMesh->mNumFaces; f++) // 면을 표시
		{
			aiFace& face = srcMesh->mFaces[f];

			for (UINT k = 0; k < face.mNumIndices; k++)
				mesh->Indices.push_back(face.mIndices[k] + startVertex);
		}

		meshes.push_back(mesh);
	}
}

void Converter::ReadSkinData()
{
	for (UINT i = 0; i < scene->mNumMeshes; i++) // 메쉬기준으로 본하고 일치를 시킬예정
	{
		aiMesh* aiMesh = scene->mMeshes[i];
		if (aiMesh->HasBones() == false) continue; // aiMesh가 본을 가지고 있지 않을 경우


		asMesh* mesh = meshes[i];

		vector<asBoneWeights> boneWeights;
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights()); // 배열처럼 다루기 위해서 미리 지정한 만큼 크기를 할당해놓는 과정

		for (UINT b = 0; b < aiMesh->mNumBones; b++) // 메쉬에 할당되어있는 본부터 반복
		{
			aiBone* aiMeshBone = aiMesh->mBones[b];

			UINT boneIndex = 0;
			for (asBone* bone : bones)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str()) // 우리가 가지고 있는 본 이름과 메쉬로부터 가져온 본 이름과 갖다면 본 정보를 가져온다
				{
					boneIndex = bone->Index;

					break;
				}
			}//for(bone)

			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // 메쉬본안에 가중치가 있어서 추가
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId;
				float weight = aiMeshBone->mWeights[w].mWeight;

				boneWeights[index].AddWeights(boneIndex, weight);
			}
		}//for(b)


		for (UINT w = 0; w < boneWeights.size(); w++) // 가중치 전체를 가지고서 정점에 추가할 예정
		{
			boneWeights[i].Normalize(); // 가중치 총합을 1로 만들어준다

			asBlendWeight blendWeight; // asBlendWeight 최종 자료형에 데이터를 할당한다.
			boneWeights[w].GetBlendWeights(blendWeight);

			mesh->Vertices[w].BlendIndices = blendWeight.Indices;
			mesh->Vertices[w].BlendWeights = blendWeight.Weights;
		}
	}
}

void Converter::WriteMeshData(wstring savePath)
{
	/*
	* CreateFolder - 해당 경로의 폴더만 생성
	* CreateFolders - 해당 경로까지 폴더가 없으면 차례로 생성
	*/
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->UInt(bones.size()); // 몇 바이트를 읽는지 모르니까 개수만큼 먼저 써놓고 시작
	for (asBone* bone : bones) // asBone 구조체 채우기
	{
		w->Int(bone->Index);
		w->String(bone->Name);
		w->Int(bone->Parent);
		w->Matrix(bone->Transform);

		SafeDelete(bone); // 변수에 저장하면 필요없어졌으므로 삭제
	}

	w->UInt(meshes.size());
	for (asMesh* meshData : meshes)
	{
		w->String(meshData->Name);
		w->Int(meshData->BoneIndex);

		w->String(meshData->MaterialName);

		w->UInt(meshData->Vertices.size()); // 정점은 VertexData므로 한번에 쓸 예정
		w->Byte(&meshData->Vertices[0], sizeof(Model::ModelVertex) * meshData->Vertices.size());

		w->UInt(meshData->Indices.size());
		w->Byte(&meshData->Indices[0], sizeof(UINT) * meshData->Indices.size());

		SafeDelete(meshData);
	}

	w->Close();
	SafeDelete(w);
}

void Converter::ExportMaterial(wstring savePath, bool bOverwrite)
{
	savePath = L"../../_Textures/" + savePath + L".material";

	if (bOverwrite == false) // 덮어씌우지 않겠다고 지정한다는 것은 기존에 파일을 수정하지 않겠다는 것
	{
		if (Path::ExistFile(savePath) == true) // 파일이 있다면 함수 종료
			return;
	}

	ReadMaterialData();
	WriteMaterialData(savePath);
}

void Converter::ReadMaterialData()
{
	for (UINT i = 0; i < scene->mNumMaterials; i++) // 배열도 전부 씬에 붙어 있기 떄문에 수량을 가져온다
	{
		aiMaterial* srcMaterial = scene->mMaterials[i]; // 씬에 있는 매터리얼 배열에서 가져온다
		asMaterial* material = new asMaterial();

		material->Name = srcMaterial->GetName().C_Str();


		// 색을 불러오는 과정
		{
			aiColor3D color; // R, G, B만 존재하는 Asimp자료형

			srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color); // 마테리얼 키와 값으로 구성되어 있으며 키를 이용해 컬러 정보를 가져온다음 지정한 변수에 할당
			material->Ambient = Color(color.r, color.g, color.b, 1.0f); // 마테리얼에 위에서 할당받은 컬러 값을 할당

			srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			material->Diffuse = Color(color.r, color.g, color.b, 1.0f);

			srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
			material->Specular = Color(color.r, color.g, color.b, 1.0f);

			srcMaterial->Get(AI_MATKEY_SHININESS, material->Specular.a);

			srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			material->Emissive = Color(color.r, color.g, color.b, 1.0f);
		}

		aiString file;

		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file); // 키의 해당하는 파일의 경로명을 저장
		material->DiffuseFile = file.C_Str();

		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->SpecularFile = file.C_Str();

		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->NormalFile = file.C_Str();


		materials.push_back(material);
	}
}

void Converter::WriteMaterialData(wstring savePath)
{
	string folder = String::ToString(Path::GetDirectoryName(savePath)); //경로에서 디렉토리이름만 가져온다
	string file = String::ToString(Path::GetFileName(savePath)); // 확장자 포함 파일 이름만 가져온다

	Path::CreateFolders(folder);


	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement* root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (asMaterial* material : materials)
	{
		Xml::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);


		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);

		
		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->DiffuseFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->SpecularFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->NormalFile).c_str());
		node->LinkEndChild(element);

		
		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->Ambient.r);
		element->SetAttribute("G", material->Ambient.g);
		element->SetAttribute("B", material->Ambient.b);
		element->SetAttribute("A", material->Ambient.a);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->Diffuse.r);
		element->SetAttribute("G", material->Diffuse.g);
		element->SetAttribute("B", material->Diffuse.b);
		element->SetAttribute("A", material->Diffuse.a);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->Specular.r);
		element->SetAttribute("G", material->Specular.g);
		element->SetAttribute("B", material->Specular.b);
		element->SetAttribute("A", material->Specular.a);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->Emissive.r);
		element->SetAttribute("G", material->Emissive.g);
		element->SetAttribute("B", material->Emissive.b);
		element->SetAttribute("A", material->Emissive.a);
		node->LinkEndChild(element);

		SafeDelete(material);
	}


	document->SaveFile((folder + file).c_str());
	SafeDelete(document);
}

/*
* 경로를 가지고서 이미지를 저장하는 함수
*/
string Converter::WriteTexture(string saveFolder, string file)
{
	if (file.length() < 1) return ""; // 파일 명이 없으면 함수 종료

	string fileName = Path::GetFileName(file); // 파일명을 확장자 명 합해서 반한
	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str()); // 경로명을 주면 경로명에 내장 텍스쳐가 있으면 반한하고 없으면 NULL을 반환

	string path = "";
	if (texture != NULL) // 내장 텍스처가 있다는 소리
	{
		path = saveFolder + fileName;

		if (texture->mHeight < 1) // 이미지가 x축만 있을 경우는 데이터가 한줄로 바이트 파일(png) 형식으로 써져 있을 경우라서 그대로 저장하면 이미지 파일이 된다
		{
			BinaryWriter w;
			w.Open(String::ToWString(path));
			w.Byte(texture->pcData, texture->mWidth);
			w.Close();
		}
		else // 이미지(내장 텍스처)가 x, y축이 있을 경우
		{
			D3D11_TEXTURE2D_DESC destDesc;
			ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
			destDesc.Width = texture->mWidth;
			destDesc.Height = texture->mHeight;
			destDesc.MipLevels = 1;
			destDesc.ArraySize = 1;
			destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			destDesc.SampleDesc.Count = 1;
			destDesc.SampleDesc.Quality = 0;
			destDesc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = texture->pcData;


			ID3D11Texture2D* dest;

			HRESULT hr;
			hr = D3D::GetDevice()->CreateTexture2D(&destDesc, &subResource, &dest);
			assert(SUCCEEDED(hr));

			D3DX11SaveTextureToFileA(D3D::GetDC(), dest, D3DX11_IFF_PNG, saveFolder.c_str());
		}
	}
	else // 텍스처 파일이 별도로 있는 경우
	{
		string directory = Path::GetDirectoryName(String::ToString(this->file));
		string origin = directory + file;
		String::Replace(&origin, "\\", "/");

		if (Path::ExistFile(origin) == false)
			return "";

		path = saveFolder + fileName;
		CopyFileA(origin.c_str(), path.c_str(), FALSE);

		String::Replace(&path, "../../_Textures/", "");
	}

	return Path::GetFileName(path);
}

void Converter::ClipList(vector<wstring>* list) // 애니메이션 정보도 씬에 있다.
{
	for (UINT i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];

		list->push_back(String::ToWString(anim->mName.C_Str()));
	}
}

void Converter::ExportAnimClip(UINT index, wstring savePath)
{
	savePath = L"../../_Models/" + savePath + L".clip";

	asClip* clip = ReadClipData(scene->mAnimations[index]);
	WriteClipData(clip, savePath);
}


// 애니메이션에 있는 본 데이터
asClip* Converter::ReadClipData(aiAnimation* animation)
{
	asClip* clip = new asClip(); // 저장할 클립데이터 자료형 변수 생성
	clip->Name = animation->mName.C_Str(); // 애니메이션의 이름
	clip->FrameRate = (float)animation->mTicksPerSecond; // 애니메이션의 fps
	clip->FrameCount = (UINT)animation->mDuration + 1; // 애니메이션 프레임의 길이이고 UINT로 끊기 때문에 소수점을 추가하기위해 +1

	vector<asClipNode> aniNodeInfos;
	for (UINT i = 0; i < animation->mNumChannels; i++)// 채널이 본 정보
	{
		aiNodeAnim* aniNode = animation->mChannels[i];

		asClipNode aniNodeInfo;
		aniNodeInfo.Name = aniNode->mNodeName;

		UINT keyCount = max(aniNode->mNumPositionKeys, aniNode->mNumScalingKeys); // S, R키중에 가장 큰 키 값을 찾기위한 과정
		keyCount = max(keyCount, aniNode->mNumRotationKeys);


		for (UINT k = 0; k < keyCount; k++)
		{
			asKeyframeData frameData;

			bool bFound = false;
			UINT t = aniNodeInfo.Keyframe.size();

			if (fabsf((float)aniNode->mPositionKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				aiVectorKey key = aniNode->mPositionKeys[k];
				frameData.Time = (float)key.mTime;
				memcpy_s(&frameData.Translation, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

				bFound = true;
			}

			if (fabsf((float)aniNode->mRotationKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				aiQuatKey key = aniNode->mRotationKeys[k];
				frameData.Time = (float)key.mTime;

				// 순서가 다르기때문에 memcpy는 사용할 수 없다 Dx와 Assimp와의 순서가 달라서
				frameData.Rotation.x = key.mValue.x;
				frameData.Rotation.y = key.mValue.y;
				frameData.Rotation.z = key.mValue.z;
				frameData.Rotation.w = key.mValue.w;

				bFound = true;
			}

			if (fabsf((float)aniNode->mScalingKeys[k].mTime - (float)t) <= D3DX_16F_EPSILON)
			{
				aiVectorKey key = aniNode->mScalingKeys[k];
				frameData.Time = (float)key.mTime;
				memcpy_s(&frameData.Scale, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

				bFound = true;
			}

			if (bFound == true)
				aniNodeInfo.Keyframe.push_back(frameData);
		}//for(k)

		if (aniNodeInfo.Keyframe.size() < clip->FrameCount)
		{
			UINT count = clip->FrameCount - aniNodeInfo.Keyframe.size();
			asKeyframeData keyFrame = aniNodeInfo.Keyframe.back();

			for (UINT n = 0; n < count; n++)
				aniNodeInfo.Keyframe.push_back(keyFrame);
		}
		clip->Duration = max(clip->Duration, aniNodeInfo.Keyframe.back().Time);

		aniNodeInfos.push_back(aniNodeInfo);
	}

	ReadKeyframeData(clip, scene->mRootNode, aniNodeInfos); // 재귀함수로서 본 데이터와 key frame데이터를 가져와서 clip에 저장후 clip을 반환해준다.


	return clip;
}

// 실제 본을 가져와서 애니메이션에 있는 본과 비교
void Converter::ReadKeyframeData(asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos)
{
	asKeyframe* keyframe = new asKeyframe(); // key frame자료형을 생성
	keyframe->BoneName = node->mName.C_Str(); // 본을 불러올 때 사용했던 aiBone과 애니메이션 정보를 저장해놓은 본과 일치시켜서 해당 애니메이션의 본 정보를 할당

	/*
		애니메이션의 본 이름이 다를 경우 자동으로 리타켓팅해주는 프로그램이 있는데 두 개의 이름이 다를 경우 맞춰주는 과정을 이 함수에서 진행하여아함
	*/

	for (UINT i = 0; i < clip->FrameCount; i++)
	{
		asClipNode* asClipNode = NULL;

		for (UINT n = 0; n < aiNodeInfos.size(); n++) // 각 본에서 프레임별로 반복문을 통해 해당 본이름과 매칭되는 본 이름이 있는지 확인
		{
			if (aiNodeInfos[n].Name == node->mName) // 이름이 같은지를 확인 후 할당
			{
				asClipNode = &aiNodeInfos[n];

				break;
			}
		}//for(n)


		asKeyframeData frameData;
		if (asClipNode == NULL)
		{
			Matrix transform(node->mTransformation[0]);
			D3DXMatrixTranspose(&transform, &transform);

			frameData.Time = (float)i;
			D3DXMatrixDecompose(&frameData.Scale, &frameData.Rotation, &frameData.Translation, &transform); // 행렬의 정보를 분리하기 위해 Decompose함수를 사용하는데 D3DXMatrixDecompose함수는 회전이 쿼터니온으로 리턴
		}
		else
		{
			frameData = asClipNode->Keyframe[i]; // 매칭되어있을 경우 clip에서 keyframe을 가져와서 할당
		}

		keyframe->Transforms.push_back(frameData);
	}

	clip->Keyframes.push_back(keyframe);

	for (UINT i = 0; i < node->mNumChildren; i++) // 재귀를 통해 자식노드 함수 호출
		ReadKeyframeData(clip, node->mChildren[i], aiNodeInfos);
}

void Converter::WriteClipData(asClip* clip, wstring savePath)
{
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->String(clip->Name);
	w->Float(clip->Duration);
	w->Float(clip->FrameRate);
	w->UInt(clip->FrameCount);

	w->UInt(clip->Keyframes.size());
	for (asKeyframe* keyframe : clip->Keyframes)
	{
		w->String(keyframe->BoneName);

		w->UInt(keyframe->Transforms.size());
		w->Byte(&keyframe->Transforms[0], sizeof(asKeyframeData) * keyframe->Transforms.size());

		SafeDelete(keyframe);
	}

	w->Close();
	SafeDelete(w);
}
