#include "water.hpp"

Water::Water()
{
	refractive_index = 1.333f;
	absorbance = 100.0f;
	color = vec3(0.0f, 0.0f, 1.0f);
	m_shader_water = new ShaderGLSL("water_shader");

}

void Water::load_shaders(std::string base_path)
{
	m_shader_water->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(m_shader_water);
}