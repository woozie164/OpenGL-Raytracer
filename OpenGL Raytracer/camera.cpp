#include "camera.h"
using namespace glm;

Camera::Camera()
{
	cameraPosition = vec3(0.0f, 0.0f, 1.0f);
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;
	initialFoV = 45.0f;
	speed = 3.0f;
	mouseSpeed = 0.005f;
	scrollWheelY = 0.0f;
}


Camera::~Camera()
{

}

void Camera::Update(){

}
