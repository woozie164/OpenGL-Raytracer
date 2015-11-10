#include <glew.h>
#include "glfw3.h"
#include <iostream>
#include "shaders.h"
#include <vector>
#include "camera.h"
#include <gtc/type_ptr.hpp>
#include <random>
#include <functional>
#include "SOIL\src\SOIL.h"

using namespace std;

void glfw_error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void * userParam)
{
	using namespace std;
	// Ignore certain messages
	switch (id)
	{
		// A verbose message about what type of memory was allocated for a buffer.
	case 131185:
		return;
	}

	cout << "---------------------opengl-callback-start------------" << endl;
	cout << "message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << endl;

	cout << "id: " << id << endl;
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	}
	cout << endl;
	cout << "---------------------opengl-callback-end--------------" << endl;
}

glm::vec3 RandomDir() {
	static std::default_random_engine generator;
	static std::uniform_int_distribution<int> distribution(-1000, 1000);
	static auto rng = std::bind(distribution, generator);
	return normalize(glm::vec3(rng(), rng(), rng()));
}

int main() {
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	GLFWwindow * window = glfwCreateWindow(800, 800, "OpenGL Raytracer", 0, 0);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);	
	glewInit();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	if (glDebugMessageCallback){
		glDebugMessageCallback(openglCallbackFunction, nullptr);
	} else {
		cout << "glDebugMessageCallback not available" << endl;
	}
	
	vector<ShaderInfo> shaders;
	loadShader("simple_vert.glsl", GL_VERTEX_SHADER, shaders);
	loadShader("simple_frag.glsl", GL_FRAGMENT_SHADER, shaders);
	GLuint simple = compileShaderProgram(shaders);
	glUseProgram(simple);
	shaders.clear();

	loadShader("raytracer_cs.glsl", GL_COMPUTE_SHADER, shaders);
	GLuint raytracerprog = compileShaderProgram(shaders);	

	shaders.clear();
	loadShader("raygen_cs.glsl", GL_COMPUTE_SHADER, shaders);
	GLuint raygenprog = compileShaderProgram(shaders);	

	shaders.clear();
	loadShader("rayintersection_cs.glsl", GL_COMPUTE_SHADER, shaders);
	GLuint rayintersectprog = compileShaderProgram(shaders);

	shaders.clear();
	loadShader("raycolor_cs.glsl", GL_COMPUTE_SHADER, shaders);
	GLuint raycolorprog = compileShaderProgram(shaders);	
	
	// Create a texture that the computer shader will render into
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 800, 0, GL_RGBA, GL_FLOAT, nullptr);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	// wtf is the difference vs glGenFrameBuffers? Different inital state - which contains what?
	//glCreateFramebuffers(1, &framebuffer);	

	/* TODO: 
	Support up to 10 moving point lights where each light casts shadows.
		+Add the new uniform declarations to all the shaders
		+Debug the data that is sent to the shader
		-Fix the shadow on the backside of the spheres
		+Add some more lights
		+Write some code that moves the points light and updates the uniform buffer
			-Move the lights continously instead of jumping from one point to another
		+Add code that converts Carteesian coordinates to barycentric
		+Use the barycentric coordinates to interpolate uv-coordinates
		-Load a texture and render it on the test triangle
		-Load a mesh and render it
	Support diffuse and specular lighting with light attenuation.
	*/

	int sizeOfRay = sizeof(float) * 4 * 6;
	GLuint ssbo = 0;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfRay * 800 * 800, nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	GLuint lightBuffer = 0;
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
	float lightData[]{
		//LightPosition, padding, LightColor, padding 
		// Note: a vec3 takes up 4 floats, 3 for the vec3 and 1 float padding
		-2.0, -2.0, -2.0,		0.0,	1.0, 0.0, 0.0,	0.0,
		-7.0, -7.0, -7.0,		0.0,	0.0, 1.0, 0.0,	0.0,
		-8.0, -7.5, -8.0,		0.0,	1.0, 0.0, 0.0,	0.0,
		-3.75, -3.75, -3.75,	0.0,	0.0, 0.0, 1.0,	0.0,
		-2.16, 1.63, -2.22,		0.0,	0.5, 0.0, 0.5,	0.0,
		-2.0, -3.0, -2.0,		0.0,	1.0, 0.0, 0.0,	0.0,
		-7.0, -8.0, -7.0,		0.0,	0.0, 1.0, 0.0,	0.0,
		-8.0, -8.5, -8.0,		0.0,	1.0, 0.0, 0.0,	0.0,
		-6.3, -6.75, -4.06,		0.0,	0.0, 0.0, 1.0,	0.0,
		2.16, 1.63, 2.22,		0.0,	0.5, 0.0, 0.5,	0.0,
	};
	int num_lights = 10;
	// 8 floats per light (2 of those flots are padding) and 10 lights in total
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 8 * 10, lightData, GL_STREAM_COPY);			

	GLuint tex_2d = SOIL_load_OGL_texture
		(
		"img_test.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);
	if (tex_2d == 0) {
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
	Camera camera;
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {		
		glClear(GL_COLOR_BUFFER_BIT);
		camera.Update();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
				
		double currentTime = glfwGetTime();
		// If two seconds have elapsed
		if (currentTime > lastTime + 2.0) {
			lastTime = currentTime;
			// Give the light a new position in a random direction
			for (int i = 0; i < num_lights * 8; i += 8)
			{
				glm::vec3 lightPos(lightData[i], lightData[i + 1], lightData[i + 2]);			
				lightPos += RandomDir() * 1.0f;
				lightData[i] = lightPos.x;
				lightData[i + 1] = lightPos.y;
				lightData[i + 2] = lightPos.z;
			}
		}

		/* Raytracer stuff */	
		for (int i = 0; i < 3; i++)
		{
			GLuint currentShaderProg;
			switch (i) 
			{		
			case 0: currentShaderProg = raygenprog; break;
			case 1: /*case 3:*/ currentShaderProg = rayintersectprog; break;								
			case 2: /*case 4:*/ currentShaderProg = raycolorprog; break;
			}

			glUseProgram(currentShaderProg);
			glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindImageTexture(1, tex_2d, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_pos"), 1, glm::value_ptr(camera.getPosition()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_dir"), 1, glm::value_ptr(camera.getDirection()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_up"), 1, glm::value_ptr(camera.getUp()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_right"), 1, glm::value_ptr(camera.getRight()));			
			
			glUniform3fv(glGetUniformLocation(currentShaderProg, "light_position"), 1, glm::value_ptr(glm::vec3(-2.0f, -2.0f, -2.0f)));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "light_color"), 1, glm::value_ptr(glm::vec3(1.0f)));

			glUniform1i(glGetUniformLocation(currentShaderProg, "num_lights"), num_lights);
			
			glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 8 * 10, lightData, GL_STREAM_COPY);
			// Returns 0, but I was expecting 2 because of the layout (binding = 2) statement ...
			GLuint test = glGetUniformBlockIndex(currentShaderProg, "LightsBuffer");
			glBindBufferBase(GL_UNIFORM_BUFFER, test, lightBuffer);

			// Workgroup size is 32 x 1
			// Dispatch 25 * 32 = 800
			glDispatchCompute(25, 800, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}
		
		
		/* Rasterizer code 
		glUniformMatrix4fv(glGetUniformLocation(simple, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(simple, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
		
		
		glBegin(GL_TRIANGLES);
		glVertex3f(-0.5f, -0.5f, 1.0f);
		glVertex3f(0.5f, -0.5f, 1.0f);
		glVertex3f(0.5f, 0.5f, 1.0f);
		
		glVertex3f(-0.5f, -0.5f, 0.5f);
		glVertex3f(0.5f, -0.5f, 0.5f);
		glVertex3f(0.5f, 0.5f, 0.5f);
		
		glEnd();
		*/
		
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, 800, 800, 0, 0, 800, 800, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}