#ifndef CONTEXT_HELPER
#define CONTEXT_HELPER

#include "helpers_common.hpp"
#include <chrono>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iomanip>
#include <sstream>

#define AVG_VALUES_COUNT 60

class ShaderGLSL;
class ContextHelper {

public:
	static GLFWwindow* window;
	static glm::uvec2 resolution;
	static float time_from_start_s;//in second
	static float time_frame_s;//in second
	static bool window_resized;
	static void print_opengl_info();
	static void init_context_all(int width, int height, std::string title, int msaa_samples = 1, int enable_vsync = 1); // no MSAA
	static bool should_not_close_window();
	static void destroy_context_all();
	static void begin_frame();
	static void end_frame();
	static void add_shader_to_hot_reload(ShaderGLSL* shader);

private:
	static void hot_reload_shaders();
	static std::vector<ShaderGLSL*> m_hot_shaders_list;
	static std::string m_title;
	static std::chrono::high_resolution_clock::time_point m_start_time;
};






#endif


