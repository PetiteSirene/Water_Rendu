#ifndef SHADER_HELPER
#define SHADER_HELPER
#include "helpers_common.hpp"

struct ShaderModule {
	GLuint shader_module;
	GLenum shader_stage_type;
	std::string path_to_shader;
	std::string shader_name;
};

class ShaderGLSL {

public:
	ShaderGLSL(std::string shader_program_name);
	void add_shader(GLenum gl_shader_stage_type, std::string folder_root, std::string shader_name);
	void remove_shader_stage(GLenum gl_shader_stage_type);
	void compile_and_link_to_program();
	void use_shader_program();

private:
	bool try_compile_and_link_to_program();
	bool m_is_shader_program_linked;
	GLuint m_shader_program;
	std::string m_shader_program_name;
	std::vector<ShaderModule> m_shader;
};
#endif