#include "OpenGLTimer.h"


OpenGLTimer::OpenGLTimer()
{
	glGenQueries(1, &startQuery);
	glGenQueries(1, &endQuery);
}


OpenGLTimer::~OpenGLTimer()
{
	glDeleteQueries(1, &startQuery);
	glDeleteQueries(1, &endQuery);
}


void OpenGLTimer::Start()
{
	glQueryCounter(startQuery, GL_TIMESTAMP);
}


void OpenGLTimer::End()
{
	glQueryCounter(endQuery, GL_TIMESTAMP);
}

// Return time in milliseconds
GLuint64 OpenGLTimer::GetElapsedTime()
{
	GLint stopTimerAvailable = 0;
	while (!stopTimerAvailable)
	{
		glGetQueryObjectiv(endQuery, GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
	}
	glGetQueryObjectui64v(startQuery, GL_QUERY_RESULT, &startTime);
	glGetQueryObjectui64v(endQuery, GL_QUERY_RESULT, &endTime);
	elapsedTime = (endTime - startTime) / 1000000.0;
	return elapsedTime;
}