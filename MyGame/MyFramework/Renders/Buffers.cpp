#include "MyFramework.h"
#include "Buffers.h"

VertexBuffer::VertexBuffer(void* data, UINT count, UINT stride, UINT slot, bool bCpuWrite, bool bGpuWrite)
	: data(data), count(count), stride(stride), slot(slot)
	, bCpuWrite(bCpuWrite), bGpuWrite(bGpuWrite)
{
	/*
	* GPU�� �����͸� �Ѱܼ� �׸��� �ֵ��� ���۸� ����
	*/
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC)); // ����ü ��� �ȿ� ������ ������ ���� �� ����ϴ� �ʱ�ȭ �Լ�
	desc.ByteWidth = stride * count; // �׷��� ����
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ������ �⸣�� VERTEX BUFFER ���

	/*
	* USAGE�� ���� ���ǹ����μ� CPU �� GPU�� �б�, ���� ���¸� �˸�
	* �� ���¿� ���� ������ �ӵ��� ���̰� ����
	* Immutable - GPU �б⸸ �����ϰ� ������������ ���� �����ϱ� ������ ���� ���� �� �Էµ� ������ ���� �Ұ����ϱ� ������ ������ ������ ���� ��� �Ұ����ϰ� ���� ����
	* Dynamic - CPU ����� GPU �бⰡ �����ϱ� ������ CPU�� �о GPU�� ������ ���� ����(���� ��ȯ ����)
	* Stage - CPU �б� ����, GPU �б� ���Ⱑ �� ������ ���·� ���� �ӵ��� ����
	*/
	if (bCpuWrite == false && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE; //GPU �б�
	}
	else if (bCpuWrite == true && bGpuWrite == false)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC; //CPU ����, GPU �б�
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // �������� ���, ���� ���� ��� Immutable�� �����ϰ� ���
	}
	else if (bCpuWrite == false && bGpuWrite == true) // GPU���� ó�� �� CPU�� ������ ���� ���
	{
		//CPU ���� ������ ���� - UpdateSubResource
		desc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	}

	/*
	* GetDevice() : CPU ���õ� ó��
	* GetDC() : GPU ���õ� ó��
	* �ؿ� 3���� RAM���� VRAM���� GPU�� ����ϵ��� �����ϴ� ����
	*/
	D3D11_SUBRESOURCE_DATA subResource = { 0 }; // ������ ������ ���� ����ü �ʱ�ȭ
	subResource.pSysMem = data; // ������ �׸� �������� ���� �ּ�

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
	desc.Usage = D3D11_USAGE_IMMUTABLE; // �� �� �ԷµǸ� ������ �� �� ���� ������ Immutable


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
	desc.Usage = D3D11_USAGE_DYNAMIC; // CPU���� �� �� �ְ� �ϱ����ؼ� Dynamic ���
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // CPU���� �� �� �ֵ��� ������ ���

	Check(D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer));
}

ConstantBuffer::~ConstantBuffer()
{
	SafeRelease(buffer);
}

void ConstantBuffer::Render()
{
	D3D11_MAPPED_SUBRESOURCE subResource;
	D3D::GetDC()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource); // Map�Լ��� �ϰ� �Ǹ� ù��° ���ڷ� ������ ���� ������ GPU���� ���������� ������ CPU ��� ��� �Ұ���
	{
		memcpy(subResource.pData, data, dataSize); // �̹� subResource�� ���ؼ� buffer�� ���� �ּҰ� ���ϵ� �����̹Ƿ� pData�� ���� ���� �����ͱ����� ���� �� �����͸� ����
	}
	D3D::GetDC()->Unmap(buffer, 0); // Map �Լ��� ����ϸ� GPU���� ���������Ƿ� ���� �� �ݵ�� Unmap�� �ʼ�
}
