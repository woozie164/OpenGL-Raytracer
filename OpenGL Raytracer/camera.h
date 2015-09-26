#pragma once
#include <glm.hpp>

class Camera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
	glm::vec3 direction;
	glm::vec3 right;
	glm::vec3 up;
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

	glm::mat4 getViewMatrix()		{ return viewMatrix; }
	glm::mat4 getProjectionMatrix() { return projectionMatrix;}

	glm::vec3 getPosition()	 { return cameraPosition;}
	glm::vec3 getDirection() { return direction; }
	glm::vec3 getRight()	 { return right; }
	glm::vec3 getUp()		 { return up; }
};

