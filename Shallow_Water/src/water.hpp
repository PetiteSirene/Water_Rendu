#ifndef WATER_CLASS
#define WATER_CLASS

#include"buffer_structures.hpp"
#include "helpers/helpers_common.hpp"
#include <GLFW/glfw3.h>

class Water {

public:
	Water();
	void load_shaders(std::string base_path);
	

private:

	ShaderGLSL* m_shader_water;

	Texture2D water_info[3]; //height, speed, normals;

	float refractive_index;
	float absorbance;
	vec3 color;
};

#endif