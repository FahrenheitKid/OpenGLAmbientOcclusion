/*
"Hello Triangle". Just the basics + shaders + 2 buffers
Dependencies:
GLEW and GLFW3 to start GL.
Based on OpenGL 4 Example Code.
*/

#include "OpenGLApp.h"

using namespace std;

// http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch17.html
// https://kupdf.com/download/pdf-real-time-rendering-3rd-pdf_5905fb05dc0d609450959e80_pdf


OpenGLApp* app;

// keep track of window size for things like the viewport and the mouse cursor

// camera


bool firstMouse = true;



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
// callbacks & function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processCameraInput(GLFWwindow *window);
void do_movement();




int main() 
{
	app = new OpenGLApp(); 
	app->glfw_and_glew_Init();
	glfwSetKeyCallback(app->g_window, key_callback);
	glfwSetCursorPosCallback(app->g_window, mouse_callback);
	glfwSetScrollCallback(app->g_window, scroll_callback);
	glfwSetWindowSizeCallback(app->g_window, glfw_window_size_callback);
	glfwSetErrorCallback(glfw_error_callback);
	glewInit();
	app->ConfigureShaders();

	app->jaspion = new Model("jaspion.obj");
	app->nanosuit = new Model("nanosuit.obj");


	// For speed computation
	double lastTime = glfwGetTime();


	app->ConfigureGBuffer();
	app->SSAOBuffer();
	app->GenerateKernelNoise();

	app->shaderLighting->Use();
	app->shaderLighting->setBool("EnableAO", true);


	while (!glfwWindowShouldClose(app->g_window)) {
		_update_fps_counter(app->g_window);

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

		processCameraInput(app->g_window);

		glfwPollEvents();

		do_movement();

		// wipe the drawing surface clear
		glClearColor(0.75294f, 0.84706f, 0.85098f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, app->g_gl_width, app->g_gl_height);

		// rendering

		// 1. geometry pass: render scene's geometry/color data into gbuffer
		app->GeometryPass();
		
		

		// 2. generate SSAO texture and blur to reduce noise
		app->SSAOProcessing();


		// 3. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		app->LightingPass();



		// -----------------------------------------------------------------------------------------------------

		if (GLFW_PRESS == glfwGetKey(app->g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(app->g_window, 1);
		}
		if (GLFW_PRESS == glfwGetKey(app->g_window, GLFW_KEY_O)) {
			app->shaderLighting->setBool("EnableAO", false);
		}
		if (GLFW_PRESS == glfwGetKey(app->g_window, GLFW_KEY_L)) {
			app->shaderLighting->setBool("EnableAO", true);
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(app->g_window);
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;

	if (app->keys[GLFW_KEY_L])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (app->keys[GLFW_KEY_F])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (app->keys[GLFW_KEY_Q])
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

	if (app->keys[GLFW_KEY_E])
	{
		scale = true;
		rot = false;
		trans = false;
		//scalex = 3.0;
	}

	if (app->keys[GLFW_KEY_E])
	{
		scale = true;
		rot = false;
		trans = false;
		//scalex = 3.0;
	}

	if (app->keys[GLFW_KEY_R])
	{
		scale = false;
		rot = true;
		trans = false;
	}

	if (app->keys[GLFW_KEY_T])
	{
		scale = false;
		rot = false;
		trans = true;
	}

	if (app->keys[GLFW_KEY_X]) // x++
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

	if (app->keys[GLFW_KEY_V]) // y++
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

	if (app->keys[GLFW_KEY_N]) // z++
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

	if (app->keys[GLFW_KEY_Z]) // x--
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

	if (app->keys[GLFW_KEY_C]) // y--
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

	if (app->keys[GLFW_KEY_B]) // z--
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
		app->lastX = xpos;
		app->lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - app->lastX;
	float yoffset = app->lastY - ypos; // reversed since y-coordinates go from bottom to top

	app->lastX = xpos;
	app->lastY = ypos;

	app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	app->camera.ProcessMouseScroll(yoffset);
}
void processCameraInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		app->camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		app->camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		app->camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		app->camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			app->keys[key] = true;
		else if (action == GLFW_RELEASE)
			app->keys[key] = false;
	}
}
void glfw_error_callback(int error, const char *description) {
	fputs(description, stderr);
	gl_log_err("%s\n", description);
}
// a call-back function
void glfw_window_size_callback(GLFWwindow *window, int width, int height)
{
	app->g_gl_width = width;
	app->g_gl_height = height;
	printf("width %i height %i\n", width, height);
	/* update any perspective matrices used here */
}