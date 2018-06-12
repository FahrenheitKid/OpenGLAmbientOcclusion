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
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Camera.h"

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

//shader variables
char vertex_shader[1024 * 256];
char fragment_shader[1024 * 256];
GLuint vs;
GLuint fs;
GLuint shader_programme;

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
glm::mat4 transform;
bool rotation_done = false;
bool translation_done = false;

int nbFrames = 0;
double fps = 0; // current fps
float spf = 0; // seconds per frame in milliseconds

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//functions prototype
int shadersSetup();

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

	
	

	/* OTHER STUFF GOES HERE NEXT */
	//GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };


	// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat cube_points_and_colours[] = {
		-1.0f,-1.0f,-1.0f,	 1.0f, 0.443f, 0.807f,
		-1.0f,-1.0f, 1.0f,   0.003f, 0.803f, 0.996f,
		-1.0f, 1.0f, 1.0f,   0.019f, 1.0f, 0.631f,
		1.0f, 1.0f,-1.0f,    0.725f, 0.403f, 1.0f,
		-1.0f,-1.0f,-1.0f,   1.0f, 0.984f, 0.588f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.443f, 0.807f,
		1.0f,-1.0f, 1.0f,    0.003f, 0.803f, 0.996f,
		-1.0f,-1.0f,-1.0f,   0.019f, 1.0f, 0.631f,
		1.0f,-1.0f,-1.0f,    0.725f, 0.403f, 1.0f,
		1.0f, 1.0f,-1.0f,    1.0f, 0.984f, 0.588f,
		1.0f,-1.0f,-1.0f,    1.0f, 0.443f, 0.807f,
		-1.0f,-1.0f,-1.0f,   0.003f, 0.803f, 0.996f,
		-1.0f,-1.0f,-1.0f,   0.019f, 1.0f, 0.631f,
		-1.0f, 1.0f, 1.0f,   0.725f, 0.403f, 1.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.984f, 0.588f,
		1.0f,-1.0f, 1.0f,    1.0f, 0.443f, 0.807f,
		-1.0f,-1.0f, 1.0f,   0.003f, 0.803f, 0.996f,
		-1.0f,-1.0f,-1.0f,   0.019f, 1.0f, 0.631f,
		-1.0f, 1.0f, 1.0f,   0.725f, 0.403f, 1.0f,
		-1.0f,-1.0f, 1.0f,   1.0f, 0.984f, 0.588f,
		1.0f,-1.0f, 1.0f,    1.0f, 0.443f, 0.807f,
		1.0f, 1.0f, 1.0f,    0.003f, 0.803f, 0.996f,
		1.0f,-1.0f,-1.0f,    0.019f, 1.0f, 0.631f,
		1.0f, 1.0f,-1.0f,    0.725f, 0.403f, 1.0f,
		1.0f,-1.0f,-1.0f,    1.0f, 0.984f, 0.588f,
		1.0f, 1.0f, 1.0f,    1.0f, 0.443f, 0.807f,
		1.0f,-1.0f, 1.0f,    0.003f, 0.803f, 0.996f,
		1.0f, 1.0f, 1.0f,    0.019f, 1.0f, 0.631f,
		1.0f, 1.0f,-1.0f,    0.725f, 0.403f, 1.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.984f, 0.588f,
		1.0f, 1.0f, 1.0f,    1.0f, 0.443f, 0.807f,
		-1.0f, 1.0f,-1.0f,   0.003f, 0.803f, 0.996f,
		-1.0f, 1.0f, 1.0f,   0.019f, 1.0f, 0.631f,
		1.0f, 1.0f, 1.0f,    0.725f, 0.403f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 0.984f, 0.588f,
		1.0f,-1.0f, 1.0f,    1.0f, 0.443f, 0.807f,
	};

	//GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	//points			//colours
	GLfloat points_and_colours[] = { 0.0f, 0.5f, 0.0f,	 1.00000f,  0.44314f,  0.80784f,
									0.5f, -0.5f, 0.0f,  0.00392f,  0.80392f,  0.99608f,
									-0.5f, -0.5f, 0.0f,	1.00000f,  0.98431f,  0.58824f };

	/*
	GLuint points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

	/* create a second VBO, containing the array of colours.
	note that we could also put them both into a single vertex buffer. in this
	case we would use the pointer and stride parameters of glVertexAttribPointer()
	to describe the different data layouts 
	GLuint colours_vbo;
	glGenBuffers(1, &colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colours, GL_STATIC_DRAW);
	
	*/
	

	GLuint points_and_colours_vbo;
	glGenBuffers(1, &points_and_colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_and_colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points_and_colours), points_and_colours, GL_STATIC_DRAW);

	GLuint cube_points_and_colours_vbo;
	glGenBuffers(1, &cube_points_and_colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube_points_and_colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_points_and_colours), cube_points_and_colours, GL_STATIC_DRAW);

	/* create the VAO.
	we bind each VBO in turn, and call glVertexAttribPointer() to indicate where
	the memory should be fetched for vertex shader input variables 0, and 1,
	respectively. we also have to explicitly enable both 'attribute' variables.
	'attribute' is the older name for vertex shader 'in' variables. */


	/*
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	*/
	

	// VAO com um único array interleaved e um único VBO
	GLuint double_vao;
	glGenVertexArrays(1, &double_vao);
	//bind vao to work with
	glBindVertexArray(double_vao);
	//bind vbo to work with

	glBindBuffer(GL_ARRAY_BUFFER, points_and_colours_vbo);

	//index in the layout, number of components per atribute, type of data, normalize data flag, stride (byte offset between consecutive vertex atributes), offset to the first component of the first vertex atribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	// in the case above: position is in the 0 layout, we have 3 components (x,y,z), it is a float, we dont want to normalize the values, since each element has position (x,y,z) AND colours (r,g,b) there is a stride of 6 floats per element, position is the first component so no need for an offset
	glEnableVertexAttribArray(0);

	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	// VAO com um único array interleaved e um único VBO
	GLuint cube_double_vao;
	glGenVertexArrays(1, &cube_double_vao);
	//bind vao to work with
	glBindVertexArray(cube_double_vao);
	//bind vbo to work with

	glBindBuffer(GL_ARRAY_BUFFER, cube_points_and_colours_vbo);

	//index in the layout, number of components per atribute, type of data, normalize data flag, stride (byte offset between consecutive vertex atributes), offset to the first component of the first vertex atribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	// in the case above: position is in the 0 layout, we have 3 components (x,y,z), it is a float, we dont want to normalize the values, since each element has position (x,y,z) AND colours (r,g,b) there is a stride of 6 floats per element, position is the first component so no need for an offset
	glEnableVertexAttribArray(0);

	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	shadersSetup();

	glEnable(GL_CULL_FACE); // cull face
	//glCullFace (GL_BACK); // cull back face
	//glFrontFace (GL_CW); // GL_CCW for counter clock-wise


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// For speed computation
	double lastTime = glfwGetTime();

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
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
											 // printf and reset
			//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			spf = 1000.0 / double(nbFrames);
			fps = 1000 / spf;
			//std::cout << fps << endl;
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

		//GLint modelLoc = glGetUniformLocation(shader_programme, "model");
		GLint viewLoc = glGetUniformLocation(shader_programme, "view");
		GLint projLoc = glGetUniformLocation(shader_programme, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		transformHandler();

		// wipe the drawing surface clear
		glClearColor(0.75294f, 0.84706f, 0.85098f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		glUseProgram(shader_programme);
		glBindVertexArray(cube_double_vao);
		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		// update other events like input handling

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

	//transform = glm::translate(transform, glm::vec3(transx, transy, transz));
	//transform = glm::scale(transform, glm::vec3(scalex, scaley, scalez));
	//transform = glm::rotate(transform, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
	//transform = glm::rotate(transform, glm::radians(rotx), glm::vec3(1.0f, 0.0f, 0.0f));
	//transform = glm::rotate(transform, glm::radians(roty), glm::vec3(0.0f, 1.0f, 0.0f));
	//transform = glm::rotate(transform, glm::radians(rotz), glm::vec3(0.0f, 0.0f, 1.0f));

	//get transform location from shader
	GLint transformLoc;
	transformLoc = glGetUniformLocation(shader_programme, "model");

	//pass matrix to that location in shader
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
}

// initialize and setups the shader
int shadersSetup()
{
	assert(parse_file_into_str("test_vs.glsl", vertex_shader, 1024 * 256));
	assert(parse_file_into_str("test_fs.glsl", fragment_shader, 1024 * 256));

	vs = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* p = (const GLchar*)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);

	// check for compile errors
	int params = -1;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
		print_shader_info_log(vs);
		return 1; // or exit or something
	}

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const GLchar*)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);

	// check for compile errors
	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
		print_shader_info_log(fs);
		return 1; // or exit or something
	}

	//glm::mat4 trans;
	//trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(
			stderr,
			"ERROR: could not link shader programme GL index %i\n",
			shader_programme
		);
		print_programme_info_log(shader_programme);
		return false;
	}
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

	/*
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	*/
	

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
			transform = glm::scale(transform, glm::vec3(1.0 + scale_amount, 1.0f, 1.0f));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(rot_amount), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(trans_amount, 0, 0));
		}
	}

	if (keys[GLFW_KEY_V]) // y++
	{
		if (scale)
		{
			transform = glm::scale(transform, glm::vec3(1.0f, 1.0 + scale_amount, 1.0f));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(rot_amount), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(0, trans_amount, 0));
		}
	}

	if (keys[GLFW_KEY_N]) // z++
	{
		if (scale)
		{
			transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0 + scale_amount));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(rot_amount), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(0, 0, trans_amount));
		}
	}

	if (keys[GLFW_KEY_Z]) // x--
	{
		if (scale)
		{
			transform = glm::scale(transform, glm::vec3(1.0 - scale_amount, 1.0f, 1.0f));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(-rot_amount), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(-trans_amount, 0, 0));
		}
	}

	if (keys[GLFW_KEY_C]) // y--
	{
		if (scale)
		{
			transform = glm::scale(transform, glm::vec3(1.0f, 1.0 - scale_amount, 1.0f));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(-rot_amount), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(0, -trans_amount, 0));
		}
	}

	if (keys[GLFW_KEY_B]) // z--
	{
		if (scale)
		{
			transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0 - scale_amount));
		}
		if (rot)
		{
			transform = glm::rotate(transform, glm::radians(-rot_amount), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (trans)
		{
			transform = glm::translate(transform, glm::vec3(0, 0, -trans_amount));
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