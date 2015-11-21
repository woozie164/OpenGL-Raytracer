#include <glew.h>
#include "glfw3.h"
#include <iostream>
#include "shaders.h"
#include <vector>
#include "camera.h"
#include <gtc/type_ptr.hpp>
#include "glm\gtx\compatibility.hpp"
#include <random>
#include <functional>
#include "SOIL\src\SOIL.h"
#include "OBJ.h"
#include "OpenGLTimer.h"
#include <fstream>

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

// Takes some position and texture coordiantes and moves them into an
// Shader Storage Buffer Object (SSBO)
GLuint UploadToSSBO(const VertexData * vertexData, unsigned int numVertices)
{
	vector<float> data;
	data.reserve(numVertices * 8);
	for (unsigned int i = 0; i < numVertices; i++)
	{		
		data.push_back(vertexData[i].point.x);
		data.push_back(vertexData[i].point.y);
		data.push_back(vertexData[i].point.z);
		data.push_back(0.0);

		data.push_back(vertexData[i].texCoord.x);
		data.push_back(vertexData[i].texCoord.y);
		data.push_back(0.0);
		data.push_back(0.0);
	}	

	// 3 floats for a vec3 + 1 float padding
	// then 2 floats for a vec3 + 2 floats padding
	GLuint vertexSize = sizeof(float) * 8;
	GLuint ssbo = 0;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertexSize * numVertices, data.data(), GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return ssbo;
}

void CompileRaytracerShader(int threadGroupSize, GLuint & raygenprog,
	GLuint & rayintersectprog, GLuint & raycolorprog)
{
	string threadGrpShaderFile;
	switch (threadGroupSize)
	{
	case 16:
		threadGrpShaderFile = "threadGrpSize16.glsl";
		break;
	case 32:
		threadGrpShaderFile = "threadGrpSize32.glsl";
		break;
	case 64:
		threadGrpShaderFile = "threadGrpSize64.glsl";
		break;
	default:
		throw("Unknown option threadGroupSize = " + threadGroupSize);
		break;
	}

	vector<ShaderInfo> shaders;	
	loadShader(threadGrpShaderFile, SHADER_HEADER, shaders);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("raygen_cs.glsl", GL_COMPUTE_SHADER, shaders);
	raygenprog = compileShaderProgram(shaders);

	shaders.clear();
	loadShader(threadGrpShaderFile, SHADER_HEADER, shaders);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("trace.glsl", SHADER_HEADER, shaders);
	loadShader("rayintersect_cs.glsl", GL_COMPUTE_SHADER, shaders);	
	rayintersectprog = compileShaderProgram(shaders);

	shaders.clear();
	loadShader(threadGrpShaderFile, SHADER_HEADER, shaders);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("trace.glsl", SHADER_HEADER, shaders);
	loadShader("raycol_cs.glsl", GL_COMPUTE_SHADER, shaders);	
	raycolorprog = compileShaderProgram(shaders);
}

void WriteBenchmarkResultsToCSVFile(int threadGrpSize, int screenWidth, int screenHeight,
	int passes, int numLights, int numTriangles, int rayCreationTime, int intersectionTime, int colorTime)
{	
	fstream f("benchmarkresults.csv", ios::out | ios::app);
	//f << "Thread group size,Screen resolution,Trace depth,Number of lights,Number of triangles,Ray Creation Time(ms),Intersection Time(ms),Color Time(ms),Total time (ms)\n";
	f << threadGrpSize << ',' << screenWidth << 'x' << screenHeight << ',' << passes << ','
		<< numLights << ',' << numTriangles << ',' << rayCreationTime << ',' << intersectionTime << ',' 
		<< colorTime << ',' << rayCreationTime + intersectionTime + colorTime << endl;
}

int RunRaytracer(int windowWidth, int windowHeight, int threadGroupSize, int renderPasses, int numLights, int numFrames) {
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Raytracer", 0, 0);
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
	}
	else {
		cout << "glDebugMessageCallback not available" << endl;
	}

	GLuint raygenprog, rayintersectprog, raycolorprog;
	CompileRaytracerShader(threadGroupSize, raygenprog, rayintersectprog, raycolorprog);

	// Create a texture that the computer shader will render into
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

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
	+Load a texture
	+Check that the shader can use it by rendering it somewhere
	+render it on the test triangle
	+Load a mesh
	+send mesh to GPU
	+render mesh
	+render mesh with texture
	-make things pretty
	-lightning is kinda ugly
	+reflections broken. Works in 103b74e though.
	-do performance analysis
	-write report on implementation and performance analysis
	Support diffuse and specular lighting with light attenuation.
	*/

	int sizeOfRay = sizeof(float) * 4 * 6;
	GLuint ssbo = 0;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeOfRay * windowWidth * windowHeight, nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	GLuint lightBuffer = 0;
	glGenBuffers(1, &lightBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
	float lightData[]{
		//LightPosition, padding, LightColor, padding 
		// Note: a vec3 takes up 4 floats, 3 for the vec3 and 1 float padding
		-0.1, -0.1, -0.1, 0.0, 1.0, 0.0, 0.0, 0.0,
			-7.0, -7.0, -7.0, 0.0, 0.0, 1.0, 0.0, 0.0,
			-8.0, -7.5, -8.0, 0.0, 1.0, 0.0, 0.0, 0.0,
			-3.75, -3.75, -3.75, 0.0, 0.0, 0.0, 1.0, 0.0,
			-2.16, 1.63, -2.22, 0.0, 0.5, 0.0, 0.5, 0.0,
			-2.0, -3.0, -2.0, 0.0, 1.0, 0.0, 0.0, 0.0,
			-7.0, -8.0, -7.0, 0.0, 0.0, 1.0, 0.0, 0.0,
			-8.0, -8.5, -8.0, 0.0, 1.0, 0.0, 0.0, 0.0,
			-6.3, -6.75, -4.06, 0.0, 0.0, 0.0, 1.0, 0.0,
			2.16, 1.63, 2.22, 0.0, 0.5, 0.0, 0.5, 0.0,
	};

	// 8 floats per light (2 of those flots are padding) and 10 lights in total
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 8 * 10, lightData, GL_STREAM_COPY);

	GLuint tex_2d = SOIL_load_OGL_texture
		(
		"sword/sword.png",
		//"img_test.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);
	if (tex_2d == 0) {
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
	Camera camera;
	camera.setPosition(2.77904, 0.594951, -1.95046);
	camera.setVerticalAngle(24.82);
	camera.setHorizontalAngle(-82.44);

	Mesh swordMesh;
	swordMesh.LoadFromObjFile("sword/", "sword.obj");
	//swordMesh.LoadFromObjFile("C:/Users/woozie/Dropbox/3D-programmering/bth_logo_obj_tga/", "bth.obj"); 
	GLuint swordDataHandle = UploadToSSBO(swordMesh.GetVertexData(0), swordMesh.GetVertexCount(0));
	int numVertices = swordMesh.GetVertexCount(0);

	double lastTime = glfwGetTime();
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	if (numFrames != 0) camera.cameraControlEnabled = false;
	int frame = 0;
	while (!glfwWindowShouldClose(window) && (frame <= numFrames)) {	
		glClear(GL_COLOR_BUFFER_BIT);
		camera.Update();

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			cout << "Camera pos: " << camera.getPosition().x << " "
				<< camera.getPosition().y << " " << camera.getPosition().z << endl;

			cout << "Camera angles: " << camera.getVerticalAngle() << " " << camera.getHorizontalAngle() << endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

		double currentTime = glfwGetTime();
		// If two seconds have elapsed
		if (currentTime > lastTime + 2.0) {
			lastTime = currentTime;
			/*
			// Make the lights rotate around the sword
			float s = 0;
			for (int i = 0; i < numLights * 8; i += 8)
			{
			glm::vec2 pos1(0.1f, 0.0f);
			glm::vec2 pos2(0.0f, 0.1f);
			float angle = glm::lerp(0.0f, 2 * 3.14f, s);
			glm::vec2 nextPos(pos1 * cos(angle) + pos2 * sin(angle));
			s += 0.1;
			lightData[i] = nextPos.x;
			lightData[i + 2] = nextPos.y;
			}
			*/

			// Give the light a new position in a random direction
			for (int i = 0; i < numLights * 8; i += 8)
			{
				glm::vec3 lightPos(lightData[i], lightData[i + 1], lightData[i + 2]);
				lightPos += RandomDir() * 1.0f;
				lightData[i] = lightPos.x;
				lightData[i + 1] = lightPos.y;
				lightData[i + 2] = lightPos.z;
			}

		}
		int passes = renderPasses;
		vector<int> time(3);
		/* Raytracer stuff */
		for (int i = 0; i < 3; i++)
		{
			OpenGLTimer timer;
			timer.Start();
			GLuint currentShaderProg;
			switch (i)
			{
			case 0: currentShaderProg = raygenprog; break;
			case 1: currentShaderProg = rayintersectprog; break;
			case 2:
				currentShaderProg = raycolorprog;
				if (passes > 1) {
					// One entire pass done, now start over again with the intersection stage
					i = 0;
					passes--;
				}
				break;
			}

			glUseProgram(currentShaderProg);
			glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindImageTexture(1, tex_2d, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swordDataHandle);
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_pos"), 1, glm::value_ptr(camera.getPosition()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_dir"), 1, glm::value_ptr(camera.getDirection()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_up"), 1, glm::value_ptr(camera.getUp()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_right"), 1, glm::value_ptr(camera.getRight()));

			glUniform1i(glGetUniformLocation(currentShaderProg, "windowWidth"), windowWidth);
			glUniform1i(glGetUniformLocation(currentShaderProg, "windowHeight"), windowHeight);

			glUniform3fv(glGetUniformLocation(currentShaderProg, "light_position"), 1, glm::value_ptr(glm::vec3(-2.0f, -2.0f, -2.0f)));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "light_color"), 1, glm::value_ptr(glm::vec3(1.0f)));

			glUniform1i(glGetUniformLocation(currentShaderProg, "num_vertices"), numVertices);
			glUniform1i(glGetUniformLocation(currentShaderProg, "num_lights"), numLights);

			glBindBuffer(GL_UNIFORM_BUFFER, lightBuffer);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 8 * 10, lightData, GL_STREAM_COPY);
			// Returns 0, but I was expecting 2 because of the layout (binding = 2) statement ...
			//GLuint test = glGetUniformBlockIndex(currentShaderProg, "LightsBuffer");
			glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(currentShaderProg, "LightsBuffer"), lightBuffer);

			// Workgroup size is 32 x 1
			// Dispatch 25 * 32 = 800
			// x * threadGrpSize = windowWidth			
			glDispatchCompute(ceil(windowWidth / (float)threadGroupSize), windowHeight, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			timer.End();
			time[i] += timer.GetElapsedTime();
		}

		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glfwSwapBuffers(window);
		glfwPollEvents();
		/*
		WriteBenchmarkResultsToCSVFile(threadGrpSize, windowWidth, windowHeight, passes, numLights,
		numVertices / 3, time[0], time[1], time[2]);
		*/

		if (numFrames != 0) {
			frame++;
		}
	}

	glfwTerminate();
	return 0;
}

int main(int argc, char * argv) {
	// Run raytracer with camera control and no limits on number of frames rendered before quitting.
	RunRaytracer(640, 480, 32, 2, 2, 0);


	RunRaytracer(400, 400, 32, 2, 2, 5);	
}