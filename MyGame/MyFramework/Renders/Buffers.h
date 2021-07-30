#pragma once

/*
* Wrapper Class 로서 ID3D11Buffer 인터페이스를 상속받는 클래스가 아닌 ID3D11Buffer 객체를 
* 클래스 멤버 변수로 두고 여러가지를 추가하는 클래스로 제작
*/
class VertexBuffer
{
public:
	VertexBuffer(void* data, UINT count, UINT stride, UINT slot = 0, bool bCpuWrite = false, bool bGpuWrite = false);
	~VertexBuffer();

	UINT Count() { return count; }
	UINT Stride() { return stride; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();

private:
	ID3D11Buffer* buffer;

	void* data; // 정점 정보를 가지고 있는 변수로서 다양한 타입이 들어올 예정
	UINT count; // 정점의 개수
	UINT stride; // 정점 1개의 크기
	UINT slot; 

	bool bCpuWrite; // CPU에서 쓸 수 있는지 마킹하는 용도
	bool bGpuWrite; //GPU에서 쓸 수 있는지 마킹하는 용도
};

///////////////////////////////////////////////////////////////////////////////

class IndexBuffer
{
public:
	IndexBuffer(void* data, UINT count);
	~IndexBuffer();

	UINT Count() { return count; }
	ID3D11Buffer* Buffer() { return buffer; }

	void Render();

private:
	ID3D11Buffer* buffer;

	void* data;
	UINT count;
};

///////////////////////////////////////////////////////////////////////////////

/*
* ConstantBuffer - 구조체로 변수를 묶어서 GPU로 보내기 위해 사용하는 버퍼
* 쉐이더의 상단 부분에서 전역 변수 생성 후 사용하게 되면 1개의 변수가 수정될 때 다른 변수들 역시 전부 수정되기
* 때문에 변수가 늘어나면 늘어날 수록 속도가 느려지게 된다, 그래서 변수들을 구조체로 구역을 지정해 GPU로 넘겨주기 위해
* 사용하는 클래스
*/
class ConstantBuffer
{
public:
	ConstantBuffer(void* data, UINT dataSize);
	~ConstantBuffer();

	ID3D11Buffer* Buffer() { return buffer; }

	void Render(); // 데이터를 밀어주기 위해서 사용하는 함수

private:
	ID3D11Buffer* buffer;

	void* data;
	UINT dataSize;
};