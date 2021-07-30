#include "MyFramework.h"
#include "Buffers.h"

VertexBuffer::VertexBuffer(void* data, UINT count, UINT stride, UINT slot, bool bCpuWrite, bool bGpuWrite)
	: data(data), count(count), stride(stride), slot(slot)
	, bCpuWrite(bCpuWrite), bGpuWrite(bGpuWrite)
{
	/*
	* GPU로 데이터를 넘겨서 그릴수 있도록 버퍼를 생성
	*/
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC)); // 구조체 멤버 안에 포인터 변수가 있을 때 사용하는 초기화 함수
	desc.ByteWidth = stride * count; // 그려질 갯수
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 정점을 기르는 VERTEX BUFFER 사용

	/*
	* USAGE를 위한 조건문으로서 CPU 및 GPU의 읽기, 쓰기 상태를 알림
	* 각 상태에 따라 렌더링 속도의 차이가 존재
	* Immutable - GPU 읽기만 가능하고 렌더링에서만 접근 가능하기 때문에 최초 생성 시 입력된 데이터 수정 불가능하기 떄문에 정점을 움직일 때는 사용 불가능하고 가장 빠름
	* Dynamic - CPU 쓰기와 GPU 읽기가 가능하기 때문에 CPU가 읽어서 GPU로 보내는 것이 가능(정점 변환 가능)
	* Stage - CPU 읽기 쓰기, GPU 읽기 쓰기가 다 가능한 상태로 가장 속도가 느림
	*/
	if (bCpuWrite == false && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE; //GPU 읽기
	}
	else if (bCpuWrite == true && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC; //CPU 쓰기, GPU 읽기
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // 엑세스를 허용, 하지 않을 경우 Immutable과 동일하게 기능
	}
	else if (bCpuWrite == false && bGpuWrite == true) // GPU에서 처리 후 CPU로 보내기 위한 경우
	{
		//CPU 쓰기 가능한 예외 - UpdateSubResource
		desc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	}

	/*
	* GetDevice() : CPU 관련된 처리
	* GetDC() : GPU 관련된 처리
	* 밑에 3줄이 RAM에서 VRAM으로 GPU를 사용하도록 복사하는 과정
	*/
	D3D11_SUBRESOURCE_DATA subResource = { 0 }; // 포인터 변수가 없는 구조체 초기화
	subResource.pSysMem = data; // 실제로 그릴 데이터의 시작 주소

	Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &buffer));
}

VertexBuffer::~VertexBuffer()
{
	SafeRelease(buffer);
}

void VertexBuffer::Render()
{
	UINT offset = 0;
	D3D::GetDC()->IASetVertexBuffers(slot, 1, &buffer, &stride, &offset);
}

///////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer(void* data, UINT count)
	: data(data), count(count)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = sizeof(UINT) * count;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.Usage = D3D11_USAGE_IMMUTABLE; // 한 번 입력되면 수정할 일 이 없기 때문에 Immutable


	D3D11_SUBRESOURCE_DATA subResource = { 0 };
	subResource.pSysMem = data;

	Check(D3D::GetDevice()->CreateBuffer(&desc, &subResource, &buffer));
}

IndexBuffer::~IndexBuffer()
{
	SafeRelease(buffer);
}

void IndexBuffer::Render()
{
	D3D::GetDC()->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
}

///////////////////////////////////////////////////////////////////////////////

ConstantBuffer::ConstantBuffer(void* data, UINT dataSize)
	: data(data), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth = dataSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC; // CPU에서 쓸 수 있게 하기위해서 Dynamic 사용
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // CPU에서 쓸 수 있도록 엑세스 허용

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));
}

ConstantBuffer::~ConstantBuffer()
{
	SafeRelease(buffer);
}

void ConstantBuffer::Render()
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource); // Map함수를 하게 되면 첫번째 인자로 지정한 버퍼 구역이 GPU에게 묶여버리기 때문에 CPU 등에서 사용 불가능
	{
		memcpy(subResource.pData, data, dataSize); // 이미 subResource를 통해서 buffer에 시작 주소가 리턴된 상태이므로 pData를 통해 버퍼 데이터구역에 접근 후 데이터를 복사
	}
	D3D::GetDC()->Unmap(buffer, 0); // Map 함수를 사용하면 GPU에게 묶여버리므로 수정 후 반드시 Unmap이 필수
}
