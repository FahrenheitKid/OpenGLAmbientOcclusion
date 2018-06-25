#include "OpenGLApp.h"



OpenGLApp::OpenGLApp()
{
	g_gl_width = 800;
	g_gl_height = 600;
	g_window = nullptr;
	lastX = g_gl_width / 2.0;
	lastY = g_gl_height / 2.0;
	camera = Camera(glm::vec3(3.0f, 2.0f, 6.0f));
	randomFloats = std::uniform_real_distribution<GLfloat>(0.0, 1.0); // generates random floats between 0.0 and 1.0
	projection = glm::perspective(glm::radians(camera.Zoom), (float)g_gl_width / (float)g_gl_height, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();
	lightPos = glm::vec3(2.0, 4.0, -2.0);
	lightColor = glm::vec3(0.2, 0.2, 0.7);
}


OpenGLApp::~OpenGLApp()
{
}

void OpenGLApp::ConfigureShaders()
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

void OpenGLApp::glfw_and_glew_Init()
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
	

	// GLFW Options
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

}

bool OpenGLApp::start_gl()
{
	gl_log("starting GLFW %s", glfwGetVersionString());

	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/* We must specify 3.2 core if on Apple OS X -- other O/S can specify
	anything here. I defined 'APPLE' in the makefile for OS X */
#ifdef APPLE
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

	/*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
	const GLFWvidmode* vmode = glfwGetVideoMode (mon);
	g_window = glfwCreateWindow (
	vmode->width, vmode->height, "Extended GL Init", mon, NULL
	);*/

	g_window = glfwCreateWindow(g_gl_width, g_gl_height, "Extended Init.", NULL, NULL);
	if (!g_window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(g_window);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte *version = glGetString(GL_VERSION);		// version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	gl_log("renderer: %s\nversion: %s\n", renderer, version);

	return true;
}

void OpenGLApp::GenerateKernelNoise()
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

void OpenGLApp::SSAOBuffer()
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

void OpenGLApp::ConfigureGBuffer()
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

void OpenGLApp::GeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projection = glm::perspective(glm::radians(camera.Zoom), (float)g_gl_width / (float)g_gl_height, 0.1f, 1000.0f);
	view = camera.GetViewMatrix();
	model = glm::mat4();
	shaderGeometry->Use();
	shaderGeometry->setMat4("projection", projection);
	shaderGeometry->setMat4("view", view);
	shaderGeometryCube->Use();
	shaderGeometryCube->setMat4("projection", projection);
	shaderGeometryCube->setMat4("view", view);
	// room cube
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
	model = glm::scale(model, glm::vec3(10.5f, 7.5f, 10.5f));
	shaderGeometryCube->Use();
	shaderGeometryCube->setMat4("model", model);
	shaderGeometryCube->setInt("invertedNormals", 1); // invert normals as we're inside the cube
	renderCube();
	shaderGeometryCube->setInt("invertedNormals", 0);
	// 
	shaderGeometry->Use();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shaderGeometry->setMat4("model", model);
	jaspion->Draw(*shaderGeometry);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 1.7f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shaderGeometry->setMat4("model", model);
	jaspion->Draw(*shaderGeometry);

	shaderGeometryCube->Use();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-3.0f, -0.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.30f));
	shaderGeometryCube->setMat4("model", model);
	nanosuit->Draw(*shaderGeometryCube);

	shaderGeometry->Use();
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 0.0f,7.0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.30f));
	shaderGeometry->setMat4("model", model);
	nanosuit->Draw(*shaderGeometry);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLApp::LightingPass()
{
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

}

void OpenGLApp::SSAOProcessing()
{
	// ------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderSSAO->Use();
	// Send kernel + rotation 
	for (unsigned int i = 0; i < 64; ++i)
		shaderSSAO->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	shaderSSAO->setMat4("projection", projection); // isso aqui pode dar errado
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// blur SSAO texture to remove noise
	// ------------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderSSAOBlur->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
