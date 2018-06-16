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

#define GL_LOG_FILE "gl.log"

using namespace std;

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


Shader* shader;
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

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processCameraInput(GLFWwindow *window);
void transformHandler();

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

int main() {
	glfw_and_glew_Init();


	shader = new Shader();
	shader->Initialize("test_vs.glsl", "test_fs.glsl");

	jaspion = new Model("jaspion.obj");
	

	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);


	// For speed computation
	double lastTime = glfwGetTime();


	// g buffer preparation
	// configure g-buffer framebuffer
	// ------------------------------
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;
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
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_gl_width, g_gl_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_gl_width, g_gl_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// lighting info
	// -------------
	const unsigned int NR_LIGHTS = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	srand(13);
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		// calculate slightly random offsets
		float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
		float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
		// also calculate random color
		float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
		float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
		float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}
	// ----------------------------------



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

		// Camera/View transformation
		glm::mat4 view;
		view = camera.GetCustomViewMatrix();

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)g_gl_width / (float)g_gl_height, 0.1f, 100.0f);

		shader->Use();

		//GLint modelLoc = glGetUniformLocation(shader_programme, "model");
		GLint viewLoc = glGetUniformLocation(shader->m_programId, "view");
		GLint projLoc = glGetUniformLocation(shader->m_programId, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		transformHandler();

		// wipe the drawing surface clear
		glClearColor(0.75294f, 0.84706f, 0.85098f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		
		jaspion->Draw(*shader);

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
	transformLoc = glGetUniformLocation(shader->m_programId, "model");

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