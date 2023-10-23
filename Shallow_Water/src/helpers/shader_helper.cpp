#include "shader_helper.hpp"

ShaderGLSL::ShaderGLSL(std::string shader_program_name)
{
	m_shader_program = glCreateProgram();
	m_shader_program_name = shader_program_name;
	m_is_shader_program_linked = false;
	m_shader.clear();
}

void ShaderGLSL::add_shader(GLenum gl_shader_stage_type, std::string folder_root, std::string shader_name)
{
	//check that no shader of this type is already added
	for (int i = 0; i < m_shader.size(); i++)
	{
		if (m_shader[i].shader_stage_type == gl_shader_stage_type)
		{
			std::cout << "WARNING: shader of type " << gl_shader_stage_type << " already added to shader program " << m_shader_program_name << std::endl;
			std::cout << "         Ignoring shader " << shader_name << std::endl;
			return;
		}
	}
	m_is_shader_program_linked = false;
	ShaderModule shader_module;
	shader_module.shader_stage_type = gl_shader_stage_type;
	shader_module.path_to_shader = folder_root + shader_name;
	shader_module.shader_name = shader_name;
	shader_module.shader_module = glCreateShader(gl_shader_stage_type);
	DebugOpenGL::set_label(GL_SHADER, shader_module.shader_module, m_shader_program_name);
	m_shader.push_back(shader_module);

}

void ShaderGLSL::remove_shader_stage(GLenum gl_shader_stage_type)
{
	m_is_shader_program_linked = false;
	for (int i = 0; i < m_shader.size(); i++)
	{
		if (m_shader[i].shader_stage_type == gl_shader_stage_type)
		{
			glDeleteShader(m_shader[i].shader_module);
			m_shader.erase(m_shader.begin() + i);
		}
	}
}

void ShaderGLSL::compile_and_link_to_program()
{
	while (!try_compile_and_link_to_program())
	{
		pause();
	}
	return;
}

bool ShaderGLSL::try_compile_and_link_to_program()
{
	int success;
	char infoLog[512];
	std::string shader_source_code;
	std::ifstream shader_file;
	//compile shaders
	for (size_t i = 0; i < m_shader.size(); i++)
	{
		//read the shader source code from file and store it in an array of strings





		shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			shader_file.open(m_shader[i].path_to_shader);
			std::stringstream shader_stream;
			shader_stream << shader_file.rdbuf();
			shader_file.close();
			shader_source_code = shader_stream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR: Could not read " << m_shader[i].path_to_shader << std::endl;
			return false;
		}
		const char* shader_source_code_cstr = shader_source_code.c_str();
		glShaderSource(m_shader[i].shader_module, 1, &shader_source_code_cstr, nullptr);

		glCompileShader(m_shader[i].shader_module);

		glGetShaderiv(m_shader[i].shader_module, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(m_shader[i].shader_module, 512, nullptr, infoLog);
			std::cout << "ERROR: Failed to compile " << m_shader[i].shader_name << " :" << std::endl;
			std::cout << infoLog << std::endl;
			return false;
		}
	}

	//link shader program
	glDeleteProgram(m_shader_program);
	m_shader_program = glCreateProgram();
	DebugOpenGL::set_label(GL_PROGRAM, m_shader_program, m_shader_program_name);
	for (int i = 0; i < m_shader.size(); i++)
	{
		glAttachShader(m_shader_program, m_shader[i].shader_module);
	}
	glLinkProgram(m_shader_program);

	glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(m_shader_program, 512, nullptr, infoLog);
		std::cout << "ERROR: Failed to link " << m_shader_program_name << " :" << std::endl;
		std::cout << infoLog << std::endl;
		glDeleteProgram(m_shader_program);
		return false;
	}

	for (int i = 0; i < m_shader.size(); i++)
	{
		glDetachShader(m_shader_program, m_shader[i].shader_module);
	}

	std::cout << "Successfully compiled and linked " << m_shader_program_name << std::endl;
	m_is_shader_program_linked = true;


	//validate shader program
	glValidateProgram(m_shader_program);
	glGetProgramiv(m_shader_program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(m_shader_program, 512, nullptr, infoLog);
		std::cout << "WARNING: Failed to validate " << m_shader_program_name << " :" << std::endl;
		std::cout << infoLog << std::endl;
	}



	return true;
}

void ShaderGLSL::use_shader_program()
{
	if (m_is_shader_program_linked)
		glUseProgram(m_shader_program);
	else
		std::cout << "ERROR: " << m_shader_program_name << " not compiled/linked !" << std::endl;
}
