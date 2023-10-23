#ifndef GROUND_CLASS
#define GROUND_CLASS

#include"buffer_structures.hpp"
#include "helpers/helpers_common.hpp"
#include <GLFW/glfw3.h>

class Scene {//Excluding the water

public:
	Scene();
	void load_shaders(std::string base_path);

	void create_material_texture_array(std::string base_path, std::string base_name);

	void flush_tessellation_levels();

	void write_params_to_application_struct(ApplicationUboDataStructure& app_ubo);
	
	void render_scene();

	void gui(ApplicationUboDataStructure& app_ubo);

private:
	ShaderGLSL* m_shader_scene;
	ShaderGLSL* m_shader_background;

	Texture2D tex_material[5];//albedo, normal, roughness, AO, height
	VertexArrayObject m_scene_vao;//Dummy VAO to tessellation drawcall

	int m_tessellation_base_level;// Triangles resolution for scene rendering
	int m_seed;
	float m_y_offset;
	float m_tile_size;//in abstract length unit (let's say meters)
	int m_tile_x_count;
};

#endif

