#include "shaders.h"
#include <fstream>
#include <sstream>
using namespace std;
/*TODO:
- Add support for headers
*/

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
	shaderInfo.filename = filename;
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
			fprintf( stderr, "Shader compilation failed! %s\n", shaders[i].filename.c_str());

			string s;
			s.resize(shaders[i].source.size() + 1);
			GLsizei length;
			glGetShaderSource(shaderHandle, s.size(), &length, (GLchar *)s.data());
			fprintf(stderr, "%s\n", s.c_str());
			if (s != shaders[i].source) {
				fprintf(stderr, "Warning: Original shader source code and source retrieved from the driver differs.");				
			}

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
		for (unsigned int i = 0; i < shaders.size(); i++)
		{
			fprintf(stderr, "%s\n", shaders[i].filename.c_str());
		}		
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
	