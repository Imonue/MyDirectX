#pragma once
#include "Camera.h"

class Freedom : public Camera {
public:
	Freedom();
	~Freedom();

	void Update() override;

	void Speed(float move, float rotation = 2.0f);
private:
	// 이동 속도 변수
	float move = 20.0f;

	// 회전 속도 변수
	float rotation = 2.0f;
};