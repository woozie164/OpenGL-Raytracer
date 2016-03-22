#include "shaders.h"
#include <fstream>
#include <sstream>
using namespace std;

//#define PRINT_SHADER_SRC_ON_ERROR

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

int compileShaderProgram(vector<ShaderInfo> & shaders, GLuint& programHandle)
{	
	GLuint pHandle = glCreateProgram();
	if( pHandle == 0 )
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}
	vector<const GLchar *> shaderSrcPtr;

	for(unsigned int i = 0; i < shaders.size(); i++)
	{		
		shaderSrcPtr.push_back(shaders[i].source.c_str());

		// If it's a shader header, it contains code that should 
		// be added to the next shader that isn't a shader header
		if (SHADER_HEADER == shaders[i].shaderType) {				
			continue;
		}

		GLuint shaderHandle = glCreateShader(shaders[i].shaderType);
		shaders[i].shaderHandle = shaderHandle;		
		glShaderSource(shaderHandle, shaderSrcPtr.size(), shaderSrcPtr.data(), 0);
		shaderSrcPtr.clear();
		glCompileShader(shaderHandle);

		GLint result;
		glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &result );
		if( result == GL_FALSE )
		{
			fprintf( stderr, "Shader compilation failed! %s\n", shaders[i].filename.c_str());

#ifdef PRINT_SHADER_SRC_ON_ERROR
			string s;
			s.resize(shaders[i].source.size() + 1);
			GLsizei length;
			glGetShaderSource(shaderHandle, s.size(), &length, (GLchar *)s.c_str());
			fprintf(stderr, "%s\n", s.c_str());
			if (s != shaders[i].source) {
				fprintf(stderr, "Warning: Original shader source code and source retrieved from the driver differs.");				
			}
#endif
			GLint logLen;
			glGetShaderiv( shaderHandle, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(shaderHandle, logLen, &written, log);
				fprintf(stderr, "Shader log:\n%s", log);
				free(log);				
			}
			return -1;
		}

		glAttachShader(pHandle, shaderHandle);
	}

	glLinkProgram( pHandle);
	GLint status;
	glGetProgramiv( pHandle, GL_LINK_STATUS, &status );
	if( status == GL_FALSE ) 
	{
		fprintf( stderr, "Failed to link shader program!\n" );
		for (unsigned int i = 0; i < shaders.size(); i++)
		{
			fprintf(stderr, "%s\n", shaders[i].filename.c_str());
		}		
		GLint logLen;
		glGetProgramiv(pHandle, GL_INFO_LOG_LENGTH, &logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(pHandle, logLen, &written, log);
			fprintf(stderr, "Program log: \n%s", log);
			free(log);			
		}
		return -2;
	}
	else
	{
		programHandle = pHandle;
		return 0;
	}
}
	