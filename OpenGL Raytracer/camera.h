#pragma once
#include <glm.hpp>

class Camera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
	glm::vec3 direction;
	float horizontalAngle;
	float verticalAngle;
	float initialFoV;
	float speed;
	float mouseSpeed;
	float scrollWheelY;

public:
	Camera();
	~Camera();

	void Update();
};

