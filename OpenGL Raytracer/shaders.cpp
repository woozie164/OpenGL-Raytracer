#include "shaders.h"
#include <fstream>
#include <sstream>
using namespace std;

void loadShader(std::string filename, GLenum shaderType, vector<ShaderInfo> & shaders)
{
	ifstream file(filename);
	if(!file)
	{
		fprintf(stderr, "Unable to open file %s", filename.c_str());
		return;
	}
	
	stringstream buffer;
	buffer << file.rdbuf();
	
	ShaderInfo shaderInfo;
	shaderInfo.source = buffer.str();
	shaderInfo.shaderType = shaderType;
	shaders.push_back(shaderInfo);
}

GLuint compileShaderProgram(vector<ShaderInfo> & shaders)
{	
	GLuint programHandle = glCreateProgram();
	if( programHandle == 0 )
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}

	for(unsigned int i = 0; i < shaders.size(); i++)
	{
		GLuint shaderHandle = glCreateShader(shaders[i].shaderType);
		shaders[i].shaderHandle = shaderHandle;
		const GLchar * source = (GLchar *)shaders[i].source.c_str();
		glShaderSource(shaderHandle, 1, &source, 0);
		glCompileShader(shaderHandle);

		GLint result;
		glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &result );
		if( result == GL_FALSE )
		{
			fprintf( stderr, "Shader compilation failed!\n" );
			GLint logLen;
			glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(shaderHandle, logLen, &written, log);
				fprintf(stderr, "Shader log:\n%s", log);
				free(log);
				return -1;
			}
		}

		glAttachShader(programHandle, shaderHandle);
	}

	glLinkProgram( programHandle);
	GLint status;
	glGetProgramiv( programHandle, GL_LINK_STATUS, &status );
	if( status == GL_FALSE ) 
	{
		fprintf( stderr, "Failed to link shader program!\n" );
		GLint logLen;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(programHandle, logLen, &written, log);
			fprintf(stderr, "Program log: \n%s", log);
			free(log);			
		}
		return -2;
	}
	else
	{
		return programHandle;
	}
}
	