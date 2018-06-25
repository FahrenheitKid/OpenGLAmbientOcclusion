/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
\******************************************************************************/
#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdarg.h>

#define GL_LOG_FILE "gl.log"

bool start_gl();

bool restart_gl_log();

bool gl_log( const char *message, ... );

/* same as gl_log except also prints to stderr */
bool gl_log_err( const char *message, ... );

void glfw_error_callback( int error, const char *description );

void log_gl_params();

void _update_fps_counter( GLFWwindow *window );

const char *GL_type_to_string( unsigned int type );

void glfw_window_size_callback( GLFWwindow *window, int width, int height );

void print_shader_info_log( GLuint shader_index );

void print_programme_info_log( GLuint sp );

void print_all( GLuint sp );

bool is_valid( GLuint sp );

bool parse_file_into_str( const char *file_name, char *shader_str, int max_len );
void renderQuad();
void renderCube();

#endif
