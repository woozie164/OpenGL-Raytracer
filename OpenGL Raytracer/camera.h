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
	bool firstTime;
	double lastTime;

public:
	Camera();
	~Camera();

	void Update();

	glm::mat4 getViewMatrix()
	{
		return viewMatrix;
	}

	glm::mat4 getProjectionMatrix()
	{
		return projectionMatrix;
	}
};

