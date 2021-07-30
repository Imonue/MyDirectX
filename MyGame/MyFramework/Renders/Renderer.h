#pragma once

class Renderer
{
public:
	Renderer(Shader* shader);
	Renderer(wstring shaderFile);
	virtual ~Renderer();

	Shader* GetShader() { return shader; }

	UINT& Pass() { return pass; }
	void Pass(UINT val) { pass = val; }

	virtual void Update();
	virtual void Render();

	Transform* GetTransform() { return transform; }

private:
	void Initialize(); // 생성자에서 공통적으로 불러올 초기화 함수

protected:
	void Topology(D3D11_PRIMITIVE_TOPOLOGY val) { topology = val; }

protected:
	Shader* shader;

	Transform* transform;
	VertexBuffer* vertexBuffer = NULL;
	IndexBuffer* indexBuffer = NULL;

	UINT vertexCount = 0;
	UINT indexCount = 0;

private:
	bool bCreateShader = false; // 내부에서 셰이더가 생성 되었는지, 외부에서 받아온 것인지를 지정하는 역할로 셰이더 삭제 여부에 대한 판단을 위해 생성한 변수

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // 이 물체가 어떤 형태로 그려질지 지정
	UINT pass = 0;

	PerFrame* perFrame;
};