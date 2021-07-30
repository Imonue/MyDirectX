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
	desc.Time = Time::Get()->Running(); // 게임 시작부터 현재까지 흐른 시간
}

/*
* 기존에 World, View, Projection을 계산 할 때
* output.Position = mul(input.Position, World)
* output.Position = mul(output.Position, View)
* output.Position = mul(output.Position, Porjection)
* 이렇게 World, View, Projection을 각각 연산을 하였는데 World, View, Projection울 곱하고 위치를 변환하는 것과
* 결과는 동일하게 나오고 특히 각각 연산하는 것보다 곱한 후 위치를 변환하는 것은 CPU에서 한번 연산해서 보내므로
* 정점 셰이더에서 변환하는 방식보다 월등히 속도가 향상
*/
void PerFrame::Render()
{
	desc.View = Context::Get()->View();
	D3DXMatrixInverse(&desc.ViewInverse, NULL, &desc.View); // View 행렬의 역행렬은 원본값

	desc.Projection = Context::Get()->Projection();
	desc.VP = desc.View * desc.Projection;

	buffer->Render();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}
