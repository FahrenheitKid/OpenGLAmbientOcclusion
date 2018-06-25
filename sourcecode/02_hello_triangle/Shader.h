#ifndef SHADER_H
#define SHADER_H
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> 
#include <string>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class Shader
{

public:
	// the program ID
	GLuint m_programId;

	// constructor reads and builds the shader
	Shader();

	~Shader();

	bool Initialize(const char* vertexPath, const char* fragPath); //nao fazer no construtor para poder retornar bool
	// use/activate the shader
	void Use();
	// utility uniform functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;



};
//--------------------------------------------------------------------
namespace ShaderUtilities
{
	bool parse_file_into_str(const char* file_name, char* shader_str, int max_len);
	void _print_shader_info_log(GLuint shader_index);
}

#endif // !SHADER_H



