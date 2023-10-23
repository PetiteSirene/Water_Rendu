
#ifndef DEBUG_OPENGL
#define DEBUG_OPENGL
#include "helpers_common.hpp"

void print_mat4(const std::string message, const glm::mat4& matrix);
void pause();
class DebugOpenGL {

public:
	static void init_opengl_debug();


	static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, GLchar const* message,
		void const* user_param);

	static void notify(const std::string message);

	static void set_label(const GLenum identifier, const GLuint& name, const std::string label);
	//with identifier in {GL_BUFFER, GL_SHADER, GL_PROGRAM, GL_VERTEX_ARRAY, GL_QUERY, GL_PROGRAM_PIPELINE, GL_TRANSFORM_FEEDBACK, GL_SAMPLER, GL_TEXTURE, GL_RENDERBUFFER, GL_FRAMEBUFFER}

	static void push_debug_group(const std::string message);
	static void pop_debug_group();

	static int m_counter;

};

#endif
