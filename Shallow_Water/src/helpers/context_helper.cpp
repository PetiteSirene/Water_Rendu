#include "context_helper.hpp"
GLFWwindow* ContextHelper::window;
glm::uvec2 ContextHelper::resolution;
float ContextHelper::time_from_start_s;
float ContextHelper::time_frame_s;
bool ContextHelper::window_resized;
std::vector<ShaderGLSL*> ContextHelper::m_hot_shaders_list;
std::string ContextHelper::m_title;
std::chrono::high_resolution_clock::time_point ContextHelper::m_start_time;

void ContextHelper::print_opengl_info()
{
	std::cout << "OPENGL INFO: " << std::endl;
	// Get OpenGL version
	const char* version = (const char*)glGetString(GL_VERSION);
	if (version != nullptr) {
		printf("    OpenGL Version: %s\n", version);
	}
	else {
		printf("    Failed to retrieve OpenGL version.\n");
	}
	const char* renderer = (const char*)glGetString(GL_RENDERER);
	printf("    GPU model: %s\n", renderer);
	int debugFlag = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &debugFlag);
	std::cout << "    OpenGL context mode:  ";
	if (debugFlag & GL_CONTEXT_FLAG_DEBUG_BIT) {
		std::cout << "DEBUG" << std::endl;
	}
	else {
		std::cout << "RELEASE" << std::endl;
	}

	std::cout << "HARDWARE INFO FOR COMPUTE SHADERS: " << std::endl;
	GLint64 warp_size;
	glGetInteger64v(GL_SUBGROUP_SIZE_KHR, &warp_size);
	std::cout << "    Thread count per warp (or wavefront): "<< warp_size << std::endl;
	
	GLint max_local_workgroup_size;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_local_workgroup_size);
	std::cout << "    Maximum workgroup size (local_x * local_y * local_z): " << max_local_workgroup_size << std::endl;

	GLint max_smem_bytes;
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &max_smem_bytes);
	std::cout << "    Maximum shared memory available in workgroup: " << max_smem_bytes << " Bytes" << std::endl << std::endl;
	
}

void ContextHelper::init_context_all(int width, int height, std::string title,int msaa_samples, int enable_vsync)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, msaa_samples);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

#ifdef OPENGL_RELEASE_CONTEXT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
	m_title = title;
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window)
	{
		std::cout << "ERROR: Failed to create glfw window" << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);


	glfwSwapInterval(enable_vsync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "ERROR: Failed to load OpenGL extension (GLAD)" << std::endl;
		exit(EXIT_FAILURE);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
	ImGui::StyleColorsClassic();
	DebugOpenGL::init_opengl_debug();
	std::cout << "Successfully initialized OpenGL - GLFW - ImGui" << std::endl << std::endl;
	resolution.x = (GLuint)width;
	resolution.y = (GLuint)height;
	window_resized = true;
	m_start_time = std::chrono::high_resolution_clock::now();
	glEnable(GL_PRIMITIVE_RESTART);
	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
}

bool ContextHelper::should_not_close_window()
{
	return !(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
}

void ContextHelper::destroy_context_all()
{

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void ContextHelper::begin_frame()
{
	DebugOpenGL::notify("Starting new frame");
	glfwPollEvents();


	//timing computations
	static float avg_time_frame[AVG_VALUES_COUNT] = { 0.0f };
	static uint32_t frame_id = 0;
	static float sum_frame_times = 0.0f;
	static std::chrono::high_resolution_clock::time_point previous_time = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
	time_from_start_s = std::chrono::duration_cast<std::chrono::microseconds>(current_time - m_start_time).count() * 1e-6f;
	time_frame_s = std::chrono::duration_cast<std::chrono::microseconds>(current_time - previous_time).count() * 1e-6f;
	previous_time = current_time;

	sum_frame_times = sum_frame_times - avg_time_frame[frame_id] + time_frame_s;
	avg_time_frame[frame_id] = time_frame_s;
	frame_id = (frame_id + 1u) % AVG_VALUES_COUNT;

	float avg_frame_time_s = sum_frame_times / float(AVG_VALUES_COUNT);
	std::stringstream title;
	title << m_title << " : " << std::fixed << std::setprecision(1) << avg_frame_time_s*1000.0f << " ms | " << 1.0f/ avg_frame_time_s << " fps | " << time_from_start_s << " sec";
	glfwSetWindowTitle(window, title.str().c_str());




	int key_state = glfwGetKey(window, GLFW_KEY_R);
	static int key_state_previous = GLFW_RELEASE;
	if (key_state == GLFW_PRESS && key_state_previous == GLFW_RELEASE)
		hot_reload_shaders();
	key_state_previous = key_state;

	//Alt + Enter key press to go full screen
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) 
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
	}
	//Alt + Shift key press to leave full screen and reset to default sze
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
	glfwSetWindowMonitor(window, nullptr, 100, 100, 800, 600, 0);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (width != resolution.x || height != resolution.y)
	{
		resolution.x = width;
		resolution.y = height;
		window_resized = true;
	}
	else
		window_resized = false;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	DebugOpenGL::m_counter = 0;
}

void ContextHelper::end_frame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	DebugOpenGL::notify("End of frame");
}

void ContextHelper::add_shader_to_hot_reload(ShaderGLSL* shader)
{
	m_hot_shaders_list.push_back(shader);
}

void ContextHelper::hot_reload_shaders()
{
	for (size_t i = 0; i < m_hot_shaders_list.size(); i++)
		m_hot_shaders_list[i]->compile_and_link_to_program();

}
