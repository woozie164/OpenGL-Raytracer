#pragma once
#include <glew.h>
#include <iostream>
#include <vector>

struct ShaderInfo
{
	std::string source;
	GLenum shaderType;
	GLuint shaderHandle;
};

GLuint compileShaderProgram(std::vector<ShaderInfo> & shaders);
void loadShader(std::string filename, GLenum shaderType, std::vector<ShaderInfo> & shaders);