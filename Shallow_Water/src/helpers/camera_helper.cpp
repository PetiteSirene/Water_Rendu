#include "camera_helper.hpp"


ProjectionMatrix::ProjectionMatrix()
{
	m_ratio_x_over_y = -1.0f;
	m_is_perspective = true;
	m_resolution = glm::uvec2(0);
	m_params = { 0.0f };
	m_proj = glm::mat4(0.0f);
}

void ProjectionMatrix::set_viewport_resolution(glm::uvec2 res)
{
	if (res.x == 0 || res.y == 0)
		return;
	if (res.x != m_resolution.x || res.y != m_resolution.y || m_ratio_x_over_y == -1.0f)
	{
		if (m_ratio_x_over_y != -1.0f)
		{
			m_resolution = res;
			m_ratio_x_over_y = (float)(res.x) / (float)(res.y);
			if (m_is_perspective)
				set_perspective(m_params.fov_y_degree, m_params.z_near, m_params.z_far);
			else
				set_ortho_centered(m_params.size_y, m_params.z_near, m_params.z_far);

		}
		else
		{
			m_resolution = res;
			m_ratio_x_over_y = (float)(res.x) / (float)(res.y);
		}
	}
}

void ProjectionMatrix::set_perspective(float fov_y_degree, float near, float far)
{
	m_is_perspective = true;
	m_params.fov_y_degree = fov_y_degree;
	m_params.z_near = near;
	m_params.z_far = far;

	m_proj = glm::perspective(glm::radians(fov_y_degree), m_ratio_x_over_y, near, far);

}

void ProjectionMatrix::set_ortho_centered(float size_y, float near, float far)
{
	m_is_perspective = false;
	m_params.size_y = size_y;
	m_params.z_near = near;
	m_params.z_far = far;
	m_proj = glm::ortho(-size_y * m_ratio_x_over_y, size_y * m_ratio_x_over_y, -size_y, size_y, near, far);

}

FreeFlyCamera::FreeFlyCamera()
{
	m_mouse_coords = glm::vec2(0.0f);
	m_w_v = glm::mat4(1.0f);
	m_pos = glm::vec3(0.0f, 0.0f,0.0f);
	m_mouse_radians_per_pixel = 0.01f;
	m_speed_unit_sec = 1.0f;
	m_boost_multiplier = 2.0f;
	m_forward = glm::vec3(1.0f,0.0f,0.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_right = glm::vec3(0.0f, 0.0f, 1.0f);
	m_theta = 0.0f;
	m_phi = 0.0f;
	m_lock_mouse_mode = false;
}

void FreeFlyCamera::set_camera(const glm::vec3 position, float theta, float phi)
{
	m_pos = position;
	m_theta = glm::radians(theta);
	m_phi = glm::radians(phi);
	build_basis();

	double mouse_x, mouse_y;
	glfwGetCursorPos(ContextHelper::window, &mouse_x, &mouse_y);
	m_mouse_coords = glm::vec2((float)mouse_x, (float)mouse_y);
}

void FreeFlyCamera::build_basis()
{
	float ct = glm::cos(m_theta);
	float st = glm::sin(m_theta);
	float cp = glm::cos(m_phi);
	float sp = glm::sin(m_phi);

	m_forward.x = ct * cp;
	m_forward.y = sp;
	m_forward.z = st * cp;
	m_right = glm::normalize(glm::cross(m_up,m_forward));

	m_w_v = glm::lookAt(m_pos, m_pos + m_forward, m_up);
}

void FreeFlyCamera::set_params(const float mouse_degree_per_pixel, const float speed_unit_sec, const float boost_multiplier)
{
	m_mouse_radians_per_pixel = glm::radians(mouse_degree_per_pixel);
	m_speed_unit_sec = speed_unit_sec;
	m_boost_multiplier = boost_multiplier;
}

void FreeFlyCamera::flush()
{
	static int lock_key_state_previous = GLFW_RELEASE;
	int lock_key_state = glfwGetKey(ContextHelper::window, GLFW_KEY_F);
	if (lock_key_state == GLFW_PRESS && lock_key_state_previous == GLFW_RELEASE)
	{
		m_lock_mouse_mode = !m_lock_mouse_mode;
	}
	lock_key_state_previous = lock_key_state;

	glm::vec2 delta_mouse = glm::vec2(0.0f);
	double mouse_x, mouse_y;
	glfwGetCursorPos(ContextHelper::window, &mouse_x, &mouse_y);
	if (m_lock_mouse_mode)
	{
		glfwSetInputMode(ContextHelper::window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		glfwSetCursorPos(ContextHelper::window, glm::floor(0.5*(double)ContextHelper::resolution.x), glm::floor(0.5*(double)ContextHelper::resolution.y));

		delta_mouse.x = m_mouse_coords.x - (float)mouse_x;
		delta_mouse.y = m_mouse_coords.y - (float)mouse_y;
		m_mouse_coords.x = glm::floor(0.5f * (float)ContextHelper::resolution.x);
		m_mouse_coords.y = glm::floor(0.5f * (float)ContextHelper::resolution.y);
	}
	else
	{
		glfwSetInputMode(ContextHelper::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		if (glfwGetMouseButton(ContextHelper::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			delta_mouse.x = (float)mouse_x - m_mouse_coords.x;
			delta_mouse.y = (float)mouse_y - m_mouse_coords.y;
		}
		m_mouse_coords.x = (float)mouse_x;
		m_mouse_coords.y = (float)mouse_y;
	}

	m_theta += delta_mouse.x * m_mouse_radians_per_pixel;
	m_phi = glm::clamp(m_phi + delta_mouse.y * m_mouse_radians_per_pixel, -3.13f*0.5f, 3.13f*0.5f);

	float delta_pos = m_speed_unit_sec * ContextHelper::time_frame_s;
	if (glfwGetKey(ContextHelper::window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		delta_pos *= m_boost_multiplier;

	if (glfwGetKey(ContextHelper::window, GLFW_KEY_W) == GLFW_PRESS)
		m_pos += delta_pos * m_forward;
	if (glfwGetKey(ContextHelper::window, GLFW_KEY_S) == GLFW_PRESS)
		m_pos -= delta_pos * m_forward;
	if (glfwGetKey(ContextHelper::window, GLFW_KEY_A) == GLFW_PRESS)
		m_pos -= delta_pos * m_right;
	if (glfwGetKey(ContextHelper::window, GLFW_KEY_D) == GLFW_PRESS)
		m_pos += delta_pos * m_right;

	build_basis();
}




