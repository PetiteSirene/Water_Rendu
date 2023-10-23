#include "debug_opengl.hpp"
int DebugOpenGL::m_counter;

void pause() {
	std::cout << "Press Enter to retry shader linking/compilation...";
	std::cout.flush();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void DebugOpenGL::message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, GLchar const* message,
	void const* user_param) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	auto const src_str = [source]() {
		switch (source) {
		case GL_DEBUG_SOURCE_API:
			return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION:
			return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER:
			return "OTHER";
		default:
			return "UNKNOWN SOURCE";
		}
	}();

	auto const type_str = [type]() {
		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY:
			return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE:
			return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER:
			return "MARKER";
		case GL_DEBUG_TYPE_OTHER:
			return "OTHER";
		default:
			return "UNKNOWN TYPE";
		}
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW:
			return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM:
			return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH:
			return "HIGH";
		default:
			return "UNKNOWN SEVERITY";
		}
	}();
	std::cout << "Message type: " << type_str << ", Severity: " << severity_str << ", Id: " << id << std::endl
		<< "Message: " << message << std::endl;
}


void DebugOpenGL::init_opengl_debug()
{
#ifdef OPENGL_RELEASE_CONTEXT
	return;
#endif
	//glDebugMessageControl
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	glDebugMessageCallback(DebugOpenGL::message_callback, nullptr);
	m_counter = 0;
}

void DebugOpenGL::notify(const std::string message)
{
#ifdef OPENGL_RELEASE_CONTEXT
	return;
#endif
	glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, message.size(), message.c_str());

}

void DebugOpenGL::set_label(const GLenum identifier, const GLuint& name, const std::string label)
{
#ifdef OPENGL_RELEASE_CONTEXT
	return;
#endif
	glObjectLabel(identifier, name, label.size(), label.c_str());
}

void DebugOpenGL::push_debug_group(const std::string message)
{
#ifdef OPENGL_RELEASE_CONTEXT
	return;
#endif
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, DebugOpenGL::m_counter, message.size(), message.c_str());
	m_counter++;
}

void DebugOpenGL::pop_debug_group()
{
#ifdef OPENGL_RELEASE_CONTEXT
	return;
#endif
	glPopDebugGroup();
}


void print_mat4(const std::string message, const glm::mat4& matrix) {
	std::cout << message << std::endl;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			std::cout << matrix[col][row] << " ";
		}
		std::cout << std::endl;
	}
}