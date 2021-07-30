#pragma once

class CubeSky
{
public:
	CubeSky(wstring file);
	~CubeSky();

	void Update();
	void Render();

public:
	Shader* shader;
	MeshSphere* sphere;

	ID3D11ShaderResourceView* srv;
	ID3DX11EffectShaderResourceVariable* sSrv;
};