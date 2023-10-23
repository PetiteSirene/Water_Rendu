#ifndef CAMERA_HELPER
#define CAMERA_HELPER

#include "helpers_common.hpp"
#include <GLFW/glfw3.h>

struct CacheCameraParams
{
	float z_near;
	float z_far;
	float fov_y_degree;
	float size_y;
};

class ProjectionMatrix {

public:
	ProjectionMatrix();
	void set_viewport_resolution(glm::uvec2 res);
	void set_perspective(float fov_y_degree, float near, float far);
	void set_ortho_centered(float size_y, float near, float far);

	glm::mat4 m_proj;

private:
	float m_ratio_x_over_y;
	glm::uvec2 m_resolution;
	bool m_is_perspective;
	CacheCameraParams m_params;

};

class FreeFlyCamera {
public:
	FreeFlyCamera();
	void set_camera(const glm::vec3 position = glm::vec3(0.0f),float theta_deg = 90.0f, float phi_deg = 0.0f);
	void set_params(const float mouse_degree_per_pixel = 0.1f, const float speed_unit_sec = 1.0f , const float boost_multiplier = 20.0f);
	void flush();//Call once per main loop
	glm::mat4 m_w_v;//world view matrix
	glm::vec3 m_pos;
private:
	void build_basis();
	float m_theta;
	float m_phi;
	
	glm::vec3 m_forward;
	glm::vec3 m_right;
	glm::vec3 m_up;
	float m_mouse_radians_per_pixel;
	float m_speed_unit_sec;
	float m_boost_multiplier;
	glm::vec2 m_mouse_coords;
	bool m_lock_mouse_mode;
};

#endif

