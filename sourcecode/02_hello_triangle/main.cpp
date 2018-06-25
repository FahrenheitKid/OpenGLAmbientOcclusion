/*
"Hello Triangle". Just the basics + shaders + 2 buffers
Dependencies:
GLEW and GLFW3 to start GL.
Based on OpenGL 4 Example Code.
*/

#include "gl_utils.h"		// utility stuff discussed in previous tutorials is here
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>

#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include <random>
#define GL_LOG_FILE "gl.log"

using namespace std;

// http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch17.html
// https://kupdf.com/download/pdf-real-time-rendering-3rd-pdf_5905fb05dc0d609450959e80_pdf

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 800;
int g_gl_height = 600;
GLFWwindow *g_window = NULL;
bool keys[1024];

GLfloat lastX = g_gl_width / 2.0;
GLfloat lastY = g_gl_height / 2.0;



// camera
Camera camera(glm::vec3(3.0f, 2.0f, 6.0f));


bool firstMouse = true;


Shader* shaderGeometry;
Shader* shaderGeometryCube;
Shader* shaderLighting;
Shader* shaderSSAO;
Shader* shaderSSAOBlur;
Model* jaspion;

//camera movement variables
bool trans = false;
bool rot = false;
bool scale = false;

float transx = 0;
float transy = 0;
float transz = 0;

float rotx = 0;
float roty = 0;
float rotz = 0;

float scalex = 1.0;
float scaley = 1.0;
float scalez = 1.0;

float trans_amount = 0;
float rot_amount = 0;
float scale_amount = 0;

float scaleratio = 0.1;
float rotratio = 10.0f;
float transratio = 0.1f;

//transformation variables
glm::mat4 transformT;
bool rotation_done = false;
bool translation_done = false;

int nbFrames = 0;
double fps = 0; // current fps
float spf = 0; // seconds per frame in milliseconds

//baixar code maid

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
//lighting
glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);
//
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processCameraInput(GLFWwindow *window);
void transformHandler();
void renderQuad();
void renderCube();


float lerp(float a, float b, float f);


void glfw_and_glew_Init()
{
	assert(restart_gl_log());
	// all the start-up code for GLFW and GLEW is called here
	assert(start_gl());
	
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	glfwMakeContextCurrent(g_window);

	// Set the required callback functions
	glfwSetKeyCallback(g_window, key_callback);
	glfwSetCursorPosCallback(g_window, mouse_callback);
	glfwSetScrollCallback(g_window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
}

unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedo;
unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
unsigned int rboDepth;

void ConfigureGBuffer()
{
	// g buffer preparation
	// configure g-buffer framebuffer
	// ------------------------------
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_gl_width, g_gl_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_gl_width, g_gl_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_gl_width, g_gl_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_gl_width, g_gl_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int ssaoFBO, ssaoBlurFBO;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

void SSAOBuffer()
{
	// also create framebuffer to hold SSAO processing stage 
	// -----------------------------------------------------
	glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_gl_width, g_gl_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_gl_width, g_gl_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
std::default_random_engine generator;
std::vector<glm::vec3> ssaoKernel;
unsigned int noiseTexture; 
void GenerateKernelNoise()
{
	// generate sample kernel
	// ----------------------
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0;

		// scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// generate noise texture
	// ----------------------
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ConfigureShaders()
{
	shaderGeometry = new Shader();
	shaderGeometry->Initialize("ssao_geometry_VS.glsl", "ssao_geometry_FS.glsl");
	shaderGeometryCube = new Shader();
	shaderGeometryCube->Initialize("ssao_geometry_VS.glsl", "ssao_geometryCube_FS.glsl");
	shaderLighting = new Shader();
	shaderLighting->Initialize("ssao_VS.glsl", "ssao_lighting_FS.glsl");
	shaderSSAO = new Shader();
	shaderSSAO->Initialize("ssao_VS.glsl", "ssao_FS.glsl");
	shaderSSAOBlur = new Shader();
	shaderSSAOBlur->Initialize("ssao_VS.glsl", "ssao_blur_FS.glsl");
	// shader configuration
	// --------------------
	shaderLighting->Use();
	shaderLighting->setInt("gPosition", 0);
	shaderLighting->setInt("gNormal", 1);
	shaderLighting->setInt("gAlbedo", 2);
	shaderLighting->setInt("ssao", 3);
	shaderSSAO->Use();
	shaderSSAO->setInt("gPosition", 0);
	shaderSSAO->setInt("gNormal", 1);
	shaderSSAO->setInt("texNoise", 2);
	shaderSSAOBlur->Use();
	shaderSSAOBlur->setInt("ssaoInput", 0);
}

int main() {
	glfw_and_glew_Init();

	ConfigureShaders();

	jaspion = new Model("jaspion.obj");
	

	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK);
	//glPolygonMode(GL_FRONT, GL_FILL);


	// For speed computation
	double lastTime = glfwGetTime();


	ConfigureGBuffer();
	SSAOBuffer();
	GenerateKernelNoise();

	while (!glfwWindowShouldClose(g_window)) {
		_update_fps_counter(g_window);

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (deltaTime != 0)
		{
			trans_amount = transratio * deltaTime;
			rot_amount = rotratio * deltaTime;
			scale_amount = scaleratio * deltaTime;
		}

		//cout << scaleratio << endl;
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) // If last prinf() was more than 1sec ago						
		{ 		
			spf = 1000.0 / double(nbFrames);
			fps = 1000 / spf;
			nbFrames = 0;
			lastTime += 1.0;
		}

		processCameraInput(g_window);

		glfwPollEvents();

		do_movement();

		// wipe the drawing surface clear
		glClearColor(0.75294f, 0.84706f, 0.85098f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		// rendering

		// 1. geometry pass: render scene's geometry/color data into gbuffer

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)g_gl_width / (float)g_gl_height, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		shaderGeometry->Use();
		shaderGeometry->setMat4("projection", projection);
		shaderGeometry->setMat4("view", view);
		shaderGeometryCube->Use();
		shaderGeometryCube->setMat4("projection", projection);
		shaderGeometryCube->setMat4("view", view);
		// room cube
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
		shaderGeometryCube->Use();
		shaderGeometryCube->setMat4("model", model);
		shaderGeometryCube->setInt("invertedNormals", 1); // invert normals as we're inside the cube
		renderCube();
		shaderGeometryCube->setInt("invertedNormals", 0);
		// nanosuit model on the floor
		shaderGeometry->Use();
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		shaderGeometry->setMat4("model", model);
		jaspion->Draw(*shaderGeometry);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(1.0f, 2.0f, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		shaderGeometry->setMat4("model", model);
		jaspion->Draw(*shaderGeometry);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 2. generate SSAO texture
		// ------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAO->Use();
		// Send kernel + rotation 
		for (unsigned int i = 0; i < 64; ++i)
			shaderSSAO->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		shaderSSAO->setMat4("projection", projection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 3. blur SSAO texture to remove noise
		// ------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		// -----------------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLighting->Use();
		// send light relevant uniforms
		glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		shaderLighting->setVec3("light.Position", lightPosView);
		shaderLighting->setVec3("light.Color", lightColor);
		// Update attenuation parameters
		const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const float linear = 0.09;
		const float quadratic = 0.032;
		shaderLighting->setFloat("light.Linear", linear);
		shaderLighting->setFloat("light.Quadratic", quadratic);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		renderQuad();




	

		if (GLFW_PRESS == glfwGetKey(g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(g_window, 1);
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(g_window);
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

void transformHandler()
{
	// Create transformations

	//get transform location from shader
	GLint transformLoc;
	transformLoc = glGetUniformLocation(shaderGeometry->m_programId, "model");

	//pass matrix to that location in shader
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformT));
}

void processCameraInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
	{
		//transx += 1;
		//transx / get_fps_counter(g_window);
		//cam.cameraPos += cameraSpeed * cam.cameraFront;
	}

	if (keys[GLFW_KEY_S])
	{
		//cam.cameraPos -= cameraSpeed * cam.cameraFront;
	}

	if (keys[GLFW_KEY_A])
	{
		//cout << cam.cameraPos.x;
		//cam.cameraPos -= glm::normalize(glm::cross(cam.cameraFront, cam.up)) * cameraSpeed;
	}

	if (keys[GLFW_KEY_D])

	{
		//cam.cameraPos += glm::normalize(glm::cross(cam.cameraFront, cam.up)) * cameraSpeed;
	}

	if (keys[GLFW_KEY_L])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (keys[GLFW_KEY_F])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (keys[GLFW_KEY_Q])
	{
		scale = false;
		rot = false;
		trans = false;

		transx = 0;
		transy = 0;
		transz = 0;

		rotx = 0;
		roty = 0;
		rotz = 0;

		scalex = 1;
		scaley = 1;
		scalez = 1;
	}

	if (keys[GLFW_KEY_E])
	{
		scale = true;
		rot = false;
		trans = false;
		//scalex = 3.0;
	}

	if (keys[GLFW_KEY_E])
	{
		scale = true;
		rot = false;
		trans = false;
		//scalex = 3.0;
	}

	if (keys[GLFW_KEY_R])
	{
		scale = false;
		rot = true;
		trans = false;
	}

	if (keys[GLFW_KEY_T])
	{
		scale = false;
		rot = false;
		trans = true;
	}

	if (keys[GLFW_KEY_X]) // x++
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0 + scale_amount, 1.0f, 1.0f));
		}
		if (rot)
		{
			transformT= glm::rotate(transformT, glm::radians(rot_amount), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (trans)
		{
			transformT= glm::translate(transformT, glm::vec3(trans_amount, 0, 0));
		}
	}

	if (keys[GLFW_KEY_V]) // y++
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0f, 1.0 + scale_amount, 1.0f));
		}
		if (rot)
		{
			transformT= glm::rotate(transformT, glm::radians(rot_amount), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (trans)
		{
			transformT= glm::translate(transformT, glm::vec3(0, trans_amount, 0));
		}
	}

	if (keys[GLFW_KEY_N]) // z++
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0f, 1.0f, 1.0 + scale_amount));
		}
		if (rot)
		{
			transformT= glm::rotate(transformT, glm::radians(rot_amount), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (trans)
		{
			transformT= glm::translate(transformT, glm::vec3(0, 0, trans_amount));
		}
	}

	if (keys[GLFW_KEY_Z]) // x--
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0 - scale_amount, 1.0f, 1.0f));
		}
		if (rot)
		{
			transformT= glm::rotate(transformT, glm::radians(-rot_amount), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (trans)
		{
			transformT= glm::translate(transformT, glm::vec3(-trans_amount, 0, 0));
		}
	}

	if (keys[GLFW_KEY_C]) // y--
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0f, 1.0 - scale_amount, 1.0f));
		}
		if (rot)
		{
			transformT= glm::rotate(transformT, glm::radians(-rot_amount), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (trans)
		{
			transformT= glm::translate(transformT, glm::vec3(0, -trans_amount, 0));
		}
	}

	if (keys[GLFW_KEY_B]) // z--
	{
		if (scale)
		{
			transformT= glm::scale(transformT, glm::vec3(1.0f, 1.0f, 1.0 - scale_amount));
		}
		if (rot)
		{
			transformT = glm::rotate(transformT, glm::radians(-rot_amount), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (trans)
		{
			transformT = glm::translate(transformT, glm::vec3(0, 0, -trans_amount));
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
																  // front face
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																														// left face
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																														-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																																											  // right face
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
																																																								   // bottom face
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																								   1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																																						 // top face
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
																																																																						 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}