#version 330 core

uniform mat4 projection;
uniform mat4 view;

layout ( location = 0 ) in vec3 vPosition;

out vec4 position;

void main()
{
	position = vec4(vPosition, 1.0);
	
	// Doesn't work
	gl_Position = projection * view * position;
	
	// Works ok
	//gl_Position = position;
	
	
	//gl_Position = position * view * projection;
}
