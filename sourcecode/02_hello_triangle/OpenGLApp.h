#pragma once
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
#include "maths_funcs.h"
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include <random>
#define GL_LOG_FILE "gl.log"
class OpenGLApp
{
public:
	OpenGLApp();
	~OpenGLApp();

	void ConfigureShaders();
	void glfw_and_glew_Init();
	bool start_gl();

	//screen
	int g_gl_width;
	int g_gl_height;
	GLFWwindow *g_window;
	bool keys[1024];
	GLfloat lastX ;
	GLfloat lastY ;
	// camera
	Camera camera;


	bool firstMouse = true;

	Shader* shaderGeometry;
	Shader* shaderGeometryCube;
	Shader* shaderLighting;
	Shader* shaderSSAO;
	Shader* shaderSSAOBlur;

	// SSAO helpers and functions
	std::uniform_real_distribution<GLfloat> randomFloats;
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	unsigned int noiseTexture;
	
	void GenerateKernelNoise();
	// ----------------
	unsigned int ssaoFBO, ssaoBlurFBO;
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
	
	void SSAOBuffer();
	// ----------------

	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedo;
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	unsigned int rboDepth;
	
	void ConfigureGBuffer();
	// renderer
	void GeometryPass(); // render set objects defined below
	void LightingPass();
	void SSAOProcessing();
	//lighting
	glm::vec3 lightPos;
	glm::vec3 lightColor;


	Model* jaspion;
	Model* nanosuit;
	
	// shaders matrix
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
};

