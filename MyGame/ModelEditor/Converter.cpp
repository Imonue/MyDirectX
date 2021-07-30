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
	this->file = L"../../_Assets/" + file; // ���� ���� ���

	scene = importer->ReadFile
	(
		String::ToString(this->file),
		aiProcess_ConvertToLeftHanded // �⺻���� ��� 3D �𵨵��� ��������ǥ�踦 ����ϱ� ������ ��ȯ�ϴ� �ǹ�
		| aiProcess_Triangulate       // ���� �������ϴ� ������� �ﰢ���� �������� ��ȯ�ؼ� �����͸� �����ش޶�� �ǹ�
		| aiProcess_GenUVCoords       // UV��ǥ�� ��ȯ�ش޶�� �ǹ�
		| aiProcess_GenNormals		  // �ﰢ������ �븻�� �ٽ� ��ȯ�ش޶�� �ǹ�
		| aiProcess_CalcTangentSpace
	);
	assert(scene != NULL); // scene�� �ҷ��� �� ���� �����̰ų� ���� ������ ��� null�� ���
}

void Converter::ExportMesh(wstring savePath)
{
	savePath = L"../../_Models/" + savePath + L".mesh"; // ���� ���� ���

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
	// TODO: �� ���� �б�
	// TODO: �޽� ���� �б�

	asBone* bone = new asBone();
	bone->Index = index;
	bone->Parent = parent;
	bone->Name = node->mName.C_Str();


	Matrix transform(node->mTransformation[0]);
	D3DXMatrixTranspose(&bone->Transform, &transform); // ���켱�� ��켱���� ��ġ ���ִ� �Լ�

	Matrix matParent;
	if (parent < 0) // parent�� ��Ʈ�� ���
		D3DXMatrixIdentity(&matParent); // 
	else
		matParent = bones[parent]->Transform;

	bone->Transform = bone->Transform * matParent; // �θ�ŭ �����̱� ���ؼ� ��
	bones.push_back(bone); // ���� �߰��� �Ǹ� �ε����� �����Ǽ� �Ʒ� �ݺ������� �ڽ��� �ε����� �ڽĵ��� ���� ����


	ReadMeshData(node, index);

	for (UINT i = 0; i < node->mNumChildren; i++)
		ReadBoneData(node->mChildren[i], bones.size(), index); // �ڽ��� �ε����� �ڽ��� �θ�� ����
	int a = 10;
}

void Converter::ReadMeshData(aiNode* node, int bone)
{
	if (node->mNumMeshes < 1) return;

	asMesh* mesh = new asMesh();
	mesh->Name = node->mName.C_Str();
	mesh->BoneIndex = bone;

	for (UINT i = 0; i < node->mNumMeshes; i++) // scene�� �ִ� ��� mesh�� �ҷ��� ����
	{
		UINT index = node->mMeshes[i]; // mesh�� ������ �ƴ� scene�� ������ �ִ� mesh�迭���� �����ϰ� �ִ� �ε����� ��ȯ
		aiMesh* srcMesh = scene->mMeshes[index];

		aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->MaterialName = material->GetName().C_Str();


		UINT startVertex = mesh->Vertices.size(); // ������ ���� ��ȣ�� ������ �־�� �޽ð� ���� ���� �Ǿ ������ ��� �̾ ���� �����ϵ��� ������ �׾Ƴ��� ũ�⸦ �Ҵ�
		for (UINT v = 0; v < srcMesh->mNumVertices; v++) // ������ ������ŭ �ݺ�
		{
			Model::ModelVertex vertex; // ���� ���ؽ� ��ü
			memcpy(&vertex.Position, &srcMesh->mVertices[v], sizeof(Vector3)); // �޽ð� �����Ѵٸ� Position�� �׻� ����

			if (srcMesh->HasTextureCoords(0)) // �ؽ�ó ��ǥ�� ������ �ִ��� �Ǵ��ϴ� ���ǹ�
				memcpy(&vertex.Uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2)); // ���̾� ������ 2���� �迭�̹Ƿ� [0]�� �ݵ�� �Է�

			if (srcMesh->HasNormals()) // �븻 ��ǥ�� ������ �ִ����� �Ǵ�
				memcpy(&vertex.Normal, &srcMesh->mNormals[v], sizeof(Vector3));

			mesh->Vertices.push_back(vertex);
		}

		for (UINT f = 0; f < srcMesh->mNumFaces; f++) // ���� ǥ��
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
	for (UINT i = 0; i < scene->mNumMeshes; i++) // �޽��������� ���ϰ� ��ġ�� ��ų����
	{
		aiMesh* aiMesh = scene->mMeshes[i];
		if (aiMesh->HasBones() == false) continue; // aiMesh�� ���� ������ ���� ���� ���


		asMesh* mesh = meshes[i];

		vector<asBoneWeights> boneWeights;
		boneWeights.assign(mesh->Vertices.size(), asBoneWeights()); // �迭ó�� �ٷ�� ���ؼ� �̸� ������ ��ŭ ũ�⸦ �Ҵ��س��� ����

		for (UINT b = 0; b < aiMesh->mNumBones; b++) // �޽��� �Ҵ�Ǿ��ִ� ������ �ݺ�
		{
			aiBone* aiMeshBone = aiMesh->mBones[b];

			UINT boneIndex = 0;
			for (asBone* bone : bones)
			{
				if (bone->Name == (string)aiMeshBone->mName.C_Str()) // �츮�� ������ �ִ� �� �̸��� �޽��κ��� ������ �� �̸��� ���ٸ� �� ������ �����´�
				{
					boneIndex = bone->Index;

					break;
				}
			}//for(bone)

			for (UINT w = 0; w < aiMeshBone->mNumWeights; w++) // �޽����ȿ� ����ġ�� �־ �߰�
			{
				UINT index = aiMeshBone->mWeights[w].mVertexId;
				float weight = aiMeshBone->mWeights[w].mWeight;

				boneWeights[index].AddWeights(boneIndex, weight);
			}
		}//for(b)


		for (UINT w = 0; w < boneWeights.size(); w++) // ����ġ ��ü�� ������ ������ �߰��� ����
		{
			boneWeights[i].Normalize(); // ����ġ ������ 1�� ������ش�

			asBlendWeight blendWeight; // asBlendWeight ���� �ڷ����� �����͸� �Ҵ��Ѵ�.
			boneWeights[w].GetBlendWeights(blendWeight);

			mesh->Vertices[w].BlendIndices = blendWeight.Indices;
			mesh->Vertices[w].BlendWeights = blendWeight.Weights;
		}
	}
}

void Converter::WriteMeshData(wstring savePath)
{
	/*
	* CreateFolder - �ش� ����� ������ ����
	* CreateFolders - �ش� ��α��� ������ ������ ���ʷ� ����
	*/
	Path::CreateFolders(Path::GetDirectoryName(savePath));

	BinaryWriter* w = new BinaryWriter();
	w->Open(savePath);

	w->UInt(bones.size()); // �� ����Ʈ�� �д��� �𸣴ϱ� ������ŭ ���� ����� ����
	for (asBone* bone : bones) // asBone ����ü ä���
	{
		w->Int(bone->Index);
		w->String(bone->Name);
		w->Int(bone->Parent);
		w->Matrix(bone->Transform);

		SafeDelete(bone); // ������ �����ϸ� �ʿ���������Ƿ� ����
	}

	w->UInt(meshes.size());
	for (asMesh* meshData : meshes)
	{
		w->String(meshData->Name);
		w->Int(meshData->BoneIndex);

		w->String(meshData->MaterialName);

		w->UInt(meshData->Vertices.size()); // ������ VertexData�Ƿ� �ѹ��� �� ����
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

	if (bOverwrite == false) // ������� �ʰڴٰ� �����Ѵٴ� ���� ������ ������ �������� �ʰڴٴ� ��
	{
		if (Path::ExistFile(savePath) == true) // ������ �ִٸ� �Լ� ����
			return;
	}

	ReadMaterialData();
	WriteMaterialData(savePath);
}

void Converter::ReadMaterialData()
{
	for (UINT i = 0; i < scene->mNumMaterials; i++) // �迭�� ���� ���� �پ� �ֱ� ������ ������ �����´�
	{
		aiMaterial* srcMaterial = scene->mMaterials[i]; // ���� �ִ� ���͸��� �迭���� �����´�
		asMaterial* material = new asMaterial();

		material->Name = srcMaterial->GetName().C_Str();


		// ���� �ҷ����� ����
		{
			aiColor3D color; // R, G, B�� �����ϴ� Asimp�ڷ���

			srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color); // ���׸��� Ű�� ������ �����Ǿ� ������ Ű�� �̿��� �÷� ������ �����´��� ������ ������ �Ҵ�
			material->Ambient = Color(color.r, color.g, color.b, 1.0f); // ���׸��� ������ �Ҵ���� �÷� ���� �Ҵ�

			srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			material->Diffuse = Color(color.r, color.g, color.b, 1.0f);

			srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
			material->Specular = Color(color.r, color.g, color.b, 1.0f);

			srcMaterial->Get(AI_MATKEY_SHININESS, material->Specular.a);

			srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			material->Emissive = Color(color.r, color.g, color.b, 1.0f);
		}

		aiString file;

		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file); // Ű�� �ش��ϴ� ������ ��θ��� ����
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
	string folder = String::ToString(Path::GetDirectoryName(savePath)); //��ο��� ���丮�̸��� �����´�
	string file = String::ToString(Path::GetFileName(savePath)); // Ȯ���� ���� ���� �̸��� �����´�

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
* ��θ� ������ �̹����� �����ϴ� �Լ�
*/
string Converter::WriteTexture(string saveFolder, string file)
{
	if (file.length() < 1) return ""; // ���� ���� ������ �Լ� ����

	string fileName = Path::GetFileName(file); // ���ϸ��� Ȯ���� �� ���ؼ� ����
	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str()); // ��θ��� �ָ� ��θ� ���� �ؽ��İ� ������ �����ϰ� ������ NULL�� ��ȯ

	string path = "";
	if (texture != NULL) // ���� �ؽ�ó�� �ִٴ� �Ҹ�
	{
		path = saveFolder + fileName;

		if (texture->mHeight < 1) // �̹����� x�ุ ���� ���� �����Ͱ� ���ٷ� ����Ʈ ����(png) �������� ���� ���� ���� �״�� �����ϸ� �̹��� ������ �ȴ�
		{
			BinaryWriter w;
			w.Open(String::ToWString(path));
			w.Byte(texture->pcData, texture->mWidth);
			w.Close();
		}
		else // �̹���(���� �ؽ�ó)�� x, y���� ���� ���
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
	else // �ؽ�ó ������ ������ �ִ� ���
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

void Converter::ClipList(vector<wstring>* list) // �ִϸ��̼� ������ ���� �ִ�.
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


// �ִϸ��̼ǿ� �ִ� �� ������
asClip* Converter::ReadClipData(aiAnimation* animation)
{
	asClip* clip = new asClip(); // ������ Ŭ�������� �ڷ��� ���� ����
	clip->Name = animation->mName.C_Str(); // �ִϸ��̼��� �̸�
	clip->FrameRate = (float)animation->mTicksPerSecond; // �ִϸ��̼��� fps
	clip->FrameCount = (UINT)animation->mDuration + 1; // �ִϸ��̼� �������� �����̰� UINT�� ���� ������ �Ҽ����� �߰��ϱ����� +1

	vector<asClipNode> aniNodeInfos;
	for (UINT i = 0; i < animation->mNumChannels; i++)// ä���� �� ����
	{
		aiNodeAnim* aniNode = animation->mChannels[i];

		asClipNode aniNodeInfo;
		aniNodeInfo.Name = aniNode->mNodeName;

		UINT keyCount = max(aniNode->mNumPositionKeys, aniNode->mNumScalingKeys); // S, RŰ�߿� ���� ū Ű ���� ã������ ����
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

				// ������ �ٸ��⶧���� memcpy�� ����� �� ���� Dx�� Assimp���� ������ �޶�
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

	ReadKeyframeData(clip, scene->mRootNode, aniNodeInfos); // ����Լ��μ� �� �����Ϳ� key frame�����͸� �����ͼ� clip�� ������ clip�� ��ȯ���ش�.


	return clip;
}

// ���� ���� �����ͼ� �ִϸ��̼ǿ� �ִ� ���� ��
void Converter::ReadKeyframeData(asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos)
{
	asKeyframe* keyframe = new asKeyframe(); // key frame�ڷ����� ����
	keyframe->BoneName = node->mName.C_Str(); // ���� �ҷ��� �� ����ߴ� aiBone�� �ִϸ��̼� ������ �����س��� ���� ��ġ���Ѽ� �ش� �ִϸ��̼��� �� ������ �Ҵ�

	/*
		�ִϸ��̼��� �� �̸��� �ٸ� ��� �ڵ����� ��Ÿ�������ִ� ���α׷��� �ִµ� �� ���� �̸��� �ٸ� ��� �����ִ� ������ �� �Լ����� �����Ͽ�����
	*/

	for (UINT i = 0; i < clip->FrameCount; i++)
	{
		asClipNode* asClipNode = NULL;

		for (UINT n = 0; n < aiNodeInfos.size(); n++) // �� ������ �����Ӻ��� �ݺ����� ���� �ش� ���̸��� ��Ī�Ǵ� �� �̸��� �ִ��� Ȯ��
		{
			if (aiNodeInfos[n].Name == node->mName) // �̸��� �������� Ȯ�� �� �Ҵ�
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
			D3DXMatrixDecompose(&frameData.Scale, &frameData.Rotation, &frameData.Translation, &transform); // ����� ������ �и��ϱ� ���� Decompose�Լ��� ����ϴµ� D3DXMatrixDecompose�Լ��� ȸ���� ���ʹϿ����� ����
		}
		else
		{
			frameData = asClipNode->Keyframe[i]; // ��Ī�Ǿ����� ��� clip���� keyframe�� �����ͼ� �Ҵ�
		}

		keyframe->Transforms.push_back(frameData);
	}

	clip->Keyframes.push_back(keyframe);

	for (UINT i = 0; i < node->mNumChildren; i++) // ��͸� ���� �ڽĳ�� �Լ� ȣ��
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
