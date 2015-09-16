#include "glfw3.h"
#include <iostream>

void glfw_error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;
}

void main() {
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();
	GLFWwindow * window = glfwCreateWindow(800, 800, "OpenGL Raytracer", 0, 0);
	glfwMakeContextCurrent(window);
	while (!glfwWindowShouldClose(window))
	{
		// Keep running
	}
}