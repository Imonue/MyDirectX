#include "MyFramework.h"
#include "PerFrame.h"

PerFrame::PerFrame(Shader* shader)
	: shader(shader)
{
	buffer = new ConstantBuffer(&desc, sizeof(Desc));
	sBuffer = shader->AsConstantBuffer("CB_PerFrame");
}

PerFrame::~PerFrame()
{
	SafeDelete(buffer);
}

void PerFrame::Update()
{
	desc.Time = Time::Get()->Running(); // ���� ���ۺ��� ������� �帥 �ð�
}

/*
* ������ World, View, Projection�� ��� �� ��
* output.Position = mul(input.Position, World)
* output.Position = mul(output.Position, View)
* output.Position = mul(output.Position, Porjection)
* �̷��� World, View, Projection�� ���� ������ �Ͽ��µ� World, View, Projection�� ���ϰ� ��ġ�� ��ȯ�ϴ� �Ͱ�
* ����� �����ϰ� ������ Ư�� ���� �����ϴ� �ͺ��� ���� �� ��ġ�� ��ȯ�ϴ� ���� CPU���� �ѹ� �����ؼ� �����Ƿ�
* ���� ���̴����� ��ȯ�ϴ� ��ĺ��� ������ �ӵ��� ���
*/
void PerFrame::Render()
{
	desc.View = Context::Get()->View();
	D3DXMatrixInverse(&desc.ViewInverse, NULL, &desc.View); // View ����� ������� ������

	desc.Projection = Context::Get()->Projection();
	desc.VP = desc.View * desc.Projection;

	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}
