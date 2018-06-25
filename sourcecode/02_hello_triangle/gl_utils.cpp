/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| This is just a file holding some commonly-used "utility" functions to keep   |
| the main file a bit easier to read. You can might build up something like    |
| this as learn more GL. Note that you don't need much code here to do good GL.|
| If you have a big object-oriented engine then maybe you can ask yourself if  |
| it is really making life easier.                                             |
\******************************************************************************/
#include "gl_utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define GL_LOG_FILE "gl.log"
#define MAX_SHADER_LENGTH 262144

/*--------------------------------LOG FUNCTIONS-------------------------------*/
bool restart_gl_log() {
	FILE *file = fopen( GL_LOG_FILE, "w" );
	if ( !file ) {
		fprintf( stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n",
						 GL_LOG_FILE );
		return false;
	}
	time_t now = time( NULL );
	char *date = ctime( &now );
	fprintf( file, "GL_LOG_FILE log. local time %s\n", date );
	fclose( file );
	return true;
}

bool gl_log( const char *message, ... ) {
	va_list argptr;
	FILE *file = fopen( GL_LOG_FILE, "a" );
	if ( !file ) {
		fprintf( stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n",
						 GL_LOG_FILE );
		return false;
	}
	va_start( argptr, message );
	vfprintf( file, message, argptr );
	va_end( argptr );
	fclose( file );
	return true;
}

/* same as gl_log except also prints to stderr */
bool gl_log_err( const char *message, ... ) {
	va_list argptr;
	FILE *file = fopen( GL_LOG_FILE, "a" );
	if ( !file ) {
		fprintf( stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n",
						 GL_LOG_FILE );
		return false;
	}
	va_start( argptr, message );
	vfprintf( file, message, argptr );
	va_end( argptr );
	va_start( argptr, message );
	vfprintf( stderr, message, argptr );
	va_end( argptr );
	fclose( file );
	return true;
}

/*--------------------------------GLFW3 and GLEW------------------------------*/
bool start_gl() {
	return false;
}



void _update_fps_counter( GLFWwindow *window ) {
	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if ( elapsed_seconds > 0.25 ) {
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf( tmp, "opengl @ fps: %.2f", fps );
		glfwSetWindowTitle( window, tmp );
		frame_count = 0;
	}
	frame_count++;
}

/*-----------------------------------SHADERS----------------------------------*/
/* copy a shader from a plain text file into a character array */
bool parse_file_into_str( const char *file_name, char *shader_str, int max_len ) {
	FILE *file = fopen( file_name, "r" );
	if ( !file ) {
		gl_log_err( "ERROR: opening file for reading: %s\n", file_name );
		return false;
	}
	size_t cnt = fread( shader_str, 1, max_len - 1, file );
	if ( (int)cnt >= max_len - 1 ) {
		gl_log_err( "WARNING: file %s too big - truncated.\n", file_name );
	}
	if ( ferror( file ) ) {
		gl_log_err( "ERROR: reading shader file %s\n", file_name );
		fclose( file );
		return false;
	}
	// append \0 to end of file string
	shader_str[cnt] = 0;
	fclose( file );
	return true;
}

void print_shader_info_log( GLuint shader_index ) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog( shader_index, max_length, &actual_length, log );
	printf( "shader info log for GL index %i:\n%s\n", shader_index, log );
	gl_log( "shader info log for GL index %i:\n%s\n", shader_index, log );
}

bool create_shader( const char *file_name, GLuint *shader, GLenum type ) {
	gl_log( "creating shader from %s...\n", file_name );
	char shader_string[MAX_SHADER_LENGTH];
	parse_file_into_str( file_name, shader_string, MAX_SHADER_LENGTH );
	*shader = glCreateShader( type );
	const GLchar *p = (const GLchar *)shader_string;
	glShaderSource( *shader, 1, &p, NULL );
	glCompileShader( *shader );
	// check for compile errors
	int params = -1;
	glGetShaderiv( *shader, GL_COMPILE_STATUS, &params );
	if ( GL_TRUE != params ) {
		gl_log_err( "ERROR: GL shader index %i did not compile\n", *shader );
		print_shader_info_log( *shader );
		return false; // or exit or something
	}
	gl_log( "shader compiled. index %i\n", *shader );
	return true;
}

void print_programme_info_log( GLuint sp ) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog( sp, max_length, &actual_length, log );
	printf( "program info log for GL index %u:\n%s", sp, log );
	gl_log( "program info log for GL index %u:\n%s", sp, log );
}

bool is_programme_valid( GLuint sp ) {
	glValidateProgram( sp );
	GLint params = -1;
	glGetProgramiv( sp, GL_VALIDATE_STATUS, &params );
	if ( GL_TRUE != params ) {
		gl_log_err( "program %i GL_VALIDATE_STATUS = GL_FALSE\n", sp );
		print_programme_info_log( sp );
		return false;
	}
	gl_log( "program %i GL_VALIDATE_STATUS = GL_TRUE\n", sp );
	return true;
}

bool create_programme( GLuint vert, GLuint frag, GLuint *programme ) {
	*programme = glCreateProgram();
	gl_log( "created programme %u. attaching shaders %u and %u...\n", *programme,
					vert, frag );
	glAttachShader( *programme, vert );
	glAttachShader( *programme, frag );
	// link the shader programme. if binding input attributes do that before link
	glLinkProgram( *programme );
	GLint params = -1;
	glGetProgramiv( *programme, GL_LINK_STATUS, &params );
	if ( GL_TRUE != params ) {
		gl_log_err( "ERROR: could not link shader programme GL index %u\n",
								*programme );
		print_programme_info_log( *programme );
		return false;
	}
	( is_programme_valid( *programme ) );
	// delete shaders here to free memory
	glDeleteShader( vert );
	glDeleteShader( frag );
	return true;
}

GLuint create_programme_from_files( const char *vert_file_name,
																		const char *frag_file_name ) {
	GLuint vert, frag, programme;
	( create_shader( vert_file_name, &vert, GL_VERTEX_SHADER ) );
	( create_shader( frag_file_name, &frag, GL_FRAGMENT_SHADER ) );
	( create_programme( vert, frag, &programme ) );
	return programme;
}
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


