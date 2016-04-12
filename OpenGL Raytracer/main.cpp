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

const int UNLIMITED_FRAMES = 0;
int gRenderPasses;

void PrintComputeShaderLimits()
{
	int workGrpInvocations;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGrpInvocations);
	cout << "MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << workGrpInvocations << endl;

	int X, Y, Z;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &X);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &Y);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &Z);
	cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNT: " << endl
		<< " X: " << X << endl
		<< " Y: " << Y << endl
		<< " Z: " << Z << endl;		
	
	int sizeX, sizeY, sizeZ;
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &sizeX);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &sizeY);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &sizeZ);
	cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE: " << endl
		<< " X: " << sizeX << endl
		<< " Y: " << sizeY << endl
		<< " Z: " << sizeZ << endl;
}

void glfw_error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool isPressedOrRepeat = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_PAGE_UP && isPressedOrRepeat)
	{
		gRenderPasses++;
	}
	if (key == GLFW_KEY_PAGE_DOWN && isPressedOrRepeat)
	{
		if(gRenderPasses > 1) gRenderPasses--;
	}
}

void APIENTRY OpenGLCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void * userParam)
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
	ShaderInfo threadGroupShaderInfo;	
	threadGroupShaderInfo.source =
		"#version 430 core\n"
		"layout(local_size_x = ";
	threadGroupShaderInfo.source += to_string(threadGroupSize);
	threadGroupShaderInfo.source += ", local_size_y = 1) in;\n";
	threadGroupShaderInfo.shaderType = SHADER_HEADER;
	threadGroupShaderInfo.filename = "No file"; // Only used for debugging

	vector<ShaderInfo> shaders;
	shaders.push_back(threadGroupShaderInfo);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("raygen_cs.glsl", GL_COMPUTE_SHADER, shaders);	
	compileShaderProgram(shaders, raygenprog);	

	shaders.clear();
	shaders.push_back(threadGroupShaderInfo);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("trace.glsl", SHADER_HEADER, shaders);
	loadShader("rayintersect_cs.glsl", GL_COMPUTE_SHADER, shaders);	
	compileShaderProgram(shaders, rayintersectprog);
	
	shaders.clear();
	shaders.push_back(threadGroupShaderInfo);
	loadShader("definitions.glsl", SHADER_HEADER, shaders);
	loadShader("trace.glsl", SHADER_HEADER, shaders);
	loadShader("raycol_cs.glsl", GL_COMPUTE_SHADER, shaders);	
	compileShaderProgram(shaders, raycolorprog);	
}

void WriteBenchmarkResultsToCSVFile(const char * filename, int threadGrpSize, int screenWidth, int screenHeight,
	int passes, int numLights, int numTriangles, int rayCreationTime, int intersectionTime, int colorTime)
{	
	fstream f(filename, ios::out | ios::app);
	//f << "Thread group size,Screen resolution,Trace depth,Number of lights,Number of triangles,Ray Creation Time(ms),Intersection Time(ms),Color Time(ms),Total time (ms)\n";
	f	<< threadGrpSize	<< ',' << screenWidth		<< ',' 
		<< screenHeight		<< ',' << passes			<< ','
		<< numLights		<< ',' << numTriangles		<< ',' 
		<< rayCreationTime	<< ',' << intersectionTime	<< ',' 		
		<< colorTime		<< endl;
}

void PrintIfFrameBufferNotComplete(GLenum e)
{
	if (e != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (e)
		{
		case GL_FRAMEBUFFER_UNDEFINED:
			printf("GL_FRAMEBUFFER_UNDEFINED");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			printf("GL_FRAMEBUFFER_UNSUPPORTED");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
			break;
		case 0:
			printf("Something went really wrong when creating this framebuffer.");
			break;
		}		
	}
}

void CompileForwardRenderingShader(GLuint & simpleShader)
{
	vector<ShaderInfo> shaders;
	loadShader("simple_vert.glsl", GL_VERTEX_SHADER, shaders);
	loadShader("simple_frag.glsl", GL_FRAGMENT_SHADER, shaders);
	compileShaderProgram(shaders, simpleShader);
}

int RunRaytracer(int windowWidth, int windowHeight, int threadGroupSize, int renderPasses, int numLights, int numFrames, const char * benchmarkOutputFile = nullptr) {
	gRenderPasses = renderPasses;
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Raytracer", 0, 0);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, glfwKeyCallback);
	
	glewInit();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	if (glDebugMessageCallback){
		glDebugMessageCallback(OpenGLCallbackFunction, nullptr);
	} else {
		cout << "glDebugMessageCallback not available" << endl;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	PrintComputeShaderLimits();

	GLuint raygenprog, rayintersectprog, raycolorprog;
	CompileRaytracerShader(threadGroupSize, raygenprog, rayintersectprog, raycolorprog);
	
	GLuint simpleShader;
	CompileForwardRenderingShader(simpleShader);

	// Create a texture that the computer shader will render into
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint depthTex;
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);		
	PrintIfFrameBufferNotComplete(e);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint framebufferDepth;
	glGenFramebuffers(1, &framebufferDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDepth);	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	PrintIfFrameBufferNotComplete(e);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// wtf is the difference vs glGenFrameBuffers? Different inital state - which contains what?
	//glCreateFramebuffers(1, &framebuffer);	

	/* TODO:
	-make things pretty
		-lightning is kinda ugly
	-do performance analysis
	-write report on implementation and performance analysis	
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
			-0.1,	-0.1, -0.1, 0.0,	1.0, 0.0, 0.0, 0.0,
			-7.0,	-0.1, -7.0, 0.0,	0.0, 1.0, 0.0, 0.0,
			-8.0,	-0.1, -8.0, 0.0,	0.3, 0.0, 0.0, 0.0,
			-3.75,	-0.1, -3.75, 0.0,	0.0, 0.0, 1.0, 0.0,
			-2.16,	-0.1, -2.22, 0.0,	0.5, 0.0, 0.5, 0.0,
			-2.0,	-0.1, -2.0, 0.0,	0.0, 0.0, 0.3, 0.0,
			-7.0,	-0.1, -7.0, 0.0,	0.0, 0.3, 0.0, 0.0,
			-8.0,	-0.1, -8.0, 0.0,	0.4, 0.4, 0.4, 0.0,
			-6.3,	-0.1, -4.06, 0.0,	0.0, 0.2, 0.2, 0.0,
			2.16,	-0.1, 2.22, 0.0,	0.8, 0.8, 0.0, 0.0,
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
	camera.aspectRatio = static_cast<float>(windowHeight) / static_cast<float>(windowWidth);

	Mesh swordMesh;
	swordMesh.LoadFromObjFile("sword/", "sword.obj");
	//swordMesh.LoadFromObjFile("bth_logo_obj_tga/", "bth.obj"); 
	GLuint swordDataHandle = UploadToSSBO(swordMesh.GetVertexData(0), swordMesh.GetVertexCount(0));
	int numVertices = swordMesh.GetVertexCount(0);

	double lastTime = glfwGetTime();
	float lastFrame = glfwGetTime();
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// Disable the camera controls if you're running benchmark mode.
	// Just so that you don't accidently move the camera.
	if (numFrames != UNLIMITED_FRAMES) camera.cameraMouseControlEnabled = false;

	int frame = 0;
	float s = 0;

	int enterKeyLastFrame = GLFW_RELEASE;
	int tKeyPressedLastFrame = GLFW_RELEASE;
	while (!glfwWindowShouldClose(window) && ((numFrames == UNLIMITED_FRAMES) || (frame < numFrames))) {
		glClear(GL_COLOR_BUFFER_BIT);
		camera.Update();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));				

		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDepth);
		float one = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &one);

		float timeNow = glfwGetTime();
		float dt = timeNow - lastFrame;
		lastFrame = timeNow;
		/*
		// Make the lights rotate around the sword	
		for (int i = 0; i < numLights * 8; i += 8)
		{
			glm::vec2 pos1(1.0f, 0.0f);
			glm::vec2 pos2(0.0f, 1.0f);
			float angle = glm::lerp(0.0f, 2 * 3.14f, s);
			angle += i * 0.2f;
			glm::vec2 nextPos(pos1 * cos(angle) + pos2 * sin(angle));

			lightData[i] = nextPos.x;
			lightData[i + 2] = nextPos.y;
		}
		s += 0.1 * dt;
		if (s >= 1.0f) {
			//s = s - 1.0f;
			s = 0.0f;
		}
		*/

		// Manual light controls
		if (glfwGetKey(window, GLFW_KEY_J)) lightData[0] -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_L)) lightData[0] += 0.01;
		if (glfwGetKey(window, GLFW_KEY_I))	lightData[1] -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_K))	lightData[1] += 0.01;
		if (glfwGetKey(window, GLFW_KEY_O))	lightData[2] -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_P))	lightData[2] += 0.01;

		int passes = gRenderPasses;
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
			case 2: currentShaderProg = raycolorprog; break;
			}

			glUseProgram(currentShaderProg);
			glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
			glBindImageTexture(1, tex_2d, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);	
			glBindImageTexture(2, depthTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R16F);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swordDataHandle);
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_pos"), 1, glm::value_ptr(camera.getPosition()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_dir"), 1, glm::value_ptr(camera.getDirection()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_up"), 1, glm::value_ptr(camera.getUp()));
			glUniform3fv(glGetUniformLocation(currentShaderProg, "camera_right"), 1, glm::value_ptr(camera.getRight()));

			glUniform1i(glGetUniformLocation(currentShaderProg, "windowWidth"), windowWidth);
			glUniform1i(glGetUniformLocation(currentShaderProg, "windowHeight"), windowHeight);
	
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
			glMemoryBarrier(GL_ALL_BARRIER_BITS);			
			glDispatchCompute(ceil(static_cast<float>(windowWidth) / static_cast<float>(threadGroupSize)), windowHeight, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			timer.End();
			time[i] += timer.GetElapsedTime();

			if (passes > 1 && currentShaderProg == raycolorprog) {
				// One entire pass done, now start over again 
				// starting with the intersection stage
				i = 0;
				passes--;
			}
		}

		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferDepth);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(simpleShader);
		glBindBuffer(GL_ARRAY_BUFFER, lightBuffer); // Note: contains not only light positions, but also padding and light color.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glUniformMatrix4fv(glGetUniformLocation(simpleShader, "projection"), 1, false, glm::value_ptr(camera.getProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(simpleShader, "view"), 1, false, glm::value_ptr(camera.getViewMatrix()));
		glPointSize(30.0f);
		glDrawArrays(GL_POINTS, 0, numLights);

		glUseProgram(0);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);		
		glDisableVertexAttribArray(0);

		/*
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferDepth);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glRasterPos2i(0, 0);
		glCopyPixels(0, 0, windowWidth, windowHeight, GL_DEPTH);
		*/
		// TODO: Use the depthbuffer to render somekind of light position indicator.
		// Blit depthbuffer?

		glfwSwapBuffers(window);
		glfwPollEvents();
		
		if (benchmarkOutputFile)
		{
			WriteBenchmarkResultsToCSVFile(benchmarkOutputFile, threadGroupSize,
				windowWidth, windowHeight, gRenderPasses, numLights,
				numVertices / 3, time[0], time[1], time[2]);	
		}

		if (numFrames != UNLIMITED_FRAMES) {
			frame++;
		}

		int enterKeyPressed = glfwGetKey(window, GLFW_KEY_ENTER);
		if (enterKeyLastFrame == GLFW_RELEASE && enterKeyPressed == GLFW_PRESS)
		{
			CompileRaytracerShader(threadGroupSize, raygenprog, rayintersectprog, raycolorprog);
			CompileForwardRenderingShader(simpleShader);
		}
		enterKeyLastFrame = enterKeyPressed;

		int tKeyPressed = glfwGetKey(window, GLFW_KEY_T);
		if (tKeyPressedLastFrame == GLFW_RELEASE && tKeyPressed == GLFW_PRESS)
		{
			camera.cameraMouseControlEnabled = !camera.cameraMouseControlEnabled;
		}
		tKeyPressedLastFrame = tKeyPressed;
		
	}

	glfwTerminate();
	return 0;
}

int main(int argc, char * argv) {
	// Run raytracer with camera control and no limits on number of frames rendered before quitting.
	RunRaytracer(800, 800, 32, 1, 1, UNLIMITED_FRAMES);
	
	// Has weird stuff at the edges of the screen
	RunRaytracer(800, 600, 32, 1, 2, UNLIMITED_FRAMES);

	// Top part of the screen is brown. Like nothing is being rendered there.
	RunRaytracer(300, 400, 32, 1, 2, UNLIMITED_FRAMES);

	//RunRaytracer(800, 800, 32, 2, 2, 5, "numtriangles_results.csv");

	/*
	// Different resolutions
	RunRaytracer(400, 400, 32, 2, 2, 5, "resolution_results.csv");	
	RunRaytracer(800, 800, 32, 2, 2, 5, "resolution_results.csv");
	RunRaytracer(1200, 1200, 32, 2, 2, 5, "resolution_results.csv");

	// Thread group size
	RunRaytracer(800, 800, 16, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 32, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 48, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 64, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 80, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 96, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 112, 2, 2, 5, "treadgroup_results.csv");
	RunRaytracer(800, 800, 128, 2, 2, 5, "treadgroup_results.csv");

	// Number of Renderpasses
	RunRaytracer(800, 800, 32, 1, 2, 5, "renderpasses_results.csv");
	RunRaytracer(800, 800, 32, 2, 2, 5, "renderpasses_results.csv");
	RunRaytracer(800, 800, 32, 3, 2, 5, "renderpasses_results.csv");

	// Number of lights
	RunRaytracer(800, 800, 32, 2, 0, 5, "lights_results.csv");
	RunRaytracer(800, 800, 32, 2, 1, 5, "lights_results.csv");
	RunRaytracer(800, 800, 32, 2, 2, 5, "lights_results.csv");
	RunRaytracer(800, 800, 32, 2, 3, 5, "lights_results.csv");
	RunRaytracer(800, 800, 32, 2, 4, 5, "lights_results.csv");
	*/
}