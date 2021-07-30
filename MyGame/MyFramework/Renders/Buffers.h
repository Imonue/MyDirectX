#pragma once

/*
* Wrapper Class �μ� ID3D11Buffer �������̽��� ��ӹ޴� Ŭ������ �ƴ� ID3D11Buffer ��ü�� 
* Ŭ���� ��� ������ �ΰ� ���������� �߰��ϴ� Ŭ������ ����
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

	void* data; // ���� ������ ������ �ִ� �����μ� �پ��� Ÿ���� ���� ����
	UINT count; // ������ ����
	UINT stride; // ���� 1���� ũ��
	UINT slot; 

	bool bCpuWrite; // CPU���� �� �� �ִ��� ��ŷ�ϴ� �뵵
	bool bGpuWrite; //GPU���� �� �� �ִ��� ��ŷ�ϴ� �뵵
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
* ConstantBuffer - ����ü�� ������ ��� GPU�� ������ ���� ����ϴ� ����
* ���̴��� ��� �κп��� ���� ���� ���� �� ����ϰ� �Ǹ� 1���� ������ ������ �� �ٸ� ������ ���� ���� �����Ǳ�
* ������ ������ �þ�� �þ ���� �ӵ��� �������� �ȴ�, �׷��� �������� ����ü�� ������ ������ GPU�� �Ѱ��ֱ� ����
* ����ϴ� Ŭ����
*/
class ConstantBuffer
{
public:
	ConstantBuffer(void* data, UINT dataSize);
	~ConstantBuffer();

	ID3D11Buffer* Buffer() { return buffer; }

	void Render(); // �����͸� �о��ֱ� ���ؼ� ����ϴ� �Լ�

private:
	ID3D11Buffer* buffer;

	void* data;
	UINT dataSize;
};