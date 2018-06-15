#include "Shader.h"



Shader::Shader()
{

}


Shader::~Shader()
{
}
bool Shader::Initialize(const char * vertexPath, const char * fragPath)
{
	const int maxSize = 1024 * 256;
	char vertex_shader[maxSize];
	char fragment_shader[maxSize];
	int params = -1;

	GLuint vs, fs;
	const char* p;

	ShaderUtilities::parse_file_into_str(vertexPath, vertex_shader, maxSize);
	ShaderUtilities::parse_file_into_str(fragPath, fragment_shader, maxSize);

	//compile vertex shader and check for errors
	vs = glCreateShader(GL_VERTEX_SHADER);
	p = (const char*)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
		ShaderUtilities::_print_shader_info_log(vs);
		return false;
	}
	//compile fragment shader and check for errors
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const char*)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);


	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
		ShaderUtilities::_print_shader_info_log(fs);
		return false;
	}

	this->m_programId = glCreateProgram();
	glAttachShader(this->m_programId, fs);
	glAttachShader(this->m_programId, vs);
	glLinkProgram(this->m_programId);


	glDeleteShader(fs);
	glDeleteShader(vs);

	return true;
}
void Shader::Use()
{
	glUseProgram(this->m_programId);
}
void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(this->m_programId, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(this->m_programId, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(this->m_programId, name.c_str()), value);
}

//helpers
bool ShaderUtilities::parse_file_into_str(const char* file_name, char* shader_str, int max_len)
{
	FILE* file = 0;
	fopen_s(&file, file_name, "r");
	if (!file)
	{
		return false;
	}
	size_t cnt = fread(shader_str, 1, max_len - 1, file);
	if (cnt >= max_len - 1)
	{
		fclose(file);
		return false;
	}
	if (ferror(file))
	{
		fclose(file);
		return false;
	}
	// append \0 to end of file string
	shader_str[cnt] = 0;
	fclose(file);
	return true;
}
/* print errors in shader compilation */
void ShaderUtilities::_print_shader_info_log(GLuint shader_index)
{
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
	printf("shader info log for GL index %i:\n%s\n", shader_index, log);
}
//shader helpers