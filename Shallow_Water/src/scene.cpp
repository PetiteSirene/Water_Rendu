#include "scene.hpp"

Scene::Scene()
{
	m_tessellation_base_level = 16;
	m_tile_size = 4.0f;
	m_tile_x_count = 24;
	m_seed = 0;
	m_y_offset = 0.0f;
	m_shader_scene = new ShaderGLSL("scene_shader");
	m_shader_background = new ShaderGLSL("background_shader");
	flush_tessellation_levels();
}

void Scene::load_shaders(std::string base_path)
{
	m_shader_scene->add_shader(GL_VERTEX_SHADER, base_path, "shaders/scene_vs.glsl");
	m_shader_scene->add_shader(GL_TESS_EVALUATION_SHADER, base_path, "shaders/scene_tes.glsl");
	m_shader_scene->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/scene_fs.glsl");
	m_shader_scene->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(m_shader_scene);

	m_shader_background->add_shader(GL_VERTEX_SHADER, base_path, "shaders/background_vs.glsl");
	m_shader_background->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/background_fs.glsl");
	m_shader_background->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(m_shader_background);
}


void Scene::create_material_texture_array(std::string base_path, std::string base_name)
{
	tex_material[0].set_format_params({ GL_RGB8,GL_RGB,GL_UNSIGNED_BYTE,3 });
	tex_material[0].set_filtering_params();
	tex_material[0].create_from_file(base_path+"data/"+ base_name+"_albedo.png");
	tex_material[0].set_slot(TEXTURE_SLOT_ALBEDO_GROUND);

	tex_material[1].set_format_params({ GL_RGB8,GL_RGB,GL_UNSIGNED_BYTE,3 });
	tex_material[1].set_filtering_params();
	tex_material[1].create_from_file(base_path + "data/" + base_name + "_normal.png");
	tex_material[1].set_slot(TEXTURE_SLOT_NORMAL_GROUND);

	tex_material[2].set_format_params({ GL_R8,GL_RED,GL_UNSIGNED_BYTE,1 });
	tex_material[2].set_filtering_params();
	tex_material[2].create_from_file(base_path + "data/" + base_name + "_roughness.png");
	tex_material[2].set_slot(TEXTURE_SLOT_ROUGHNESS_GROUND);

	tex_material[3].set_format_params({ GL_R8,GL_RED,GL_UNSIGNED_BYTE,1 });
	tex_material[3].set_filtering_params();
	tex_material[3].create_from_file(base_path + "data/" + base_name + "_ao.png");
	tex_material[3].set_slot(TEXTURE_SLOT_AMBIENT_OCCLUSION_GROUND);

	tex_material[4].set_format_params({ GL_R8,GL_RED,GL_UNSIGNED_BYTE,1 });
	tex_material[4].set_filtering_params();
	tex_material[4].create_from_file(base_path + "data/" + base_name + "_height.png");
	tex_material[4].set_slot(TEXTURE_SLOT_DISPLACEMENT_GROUND);
}


void Scene::flush_tessellation_levels()
{
	static int tess = -1;

	if (tess != m_tessellation_base_level)
	{
		GLfloat levels[] = { m_tessellation_base_level, m_tessellation_base_level, m_tessellation_base_level, m_tessellation_base_level };
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, levels);
		tess = m_tessellation_base_level;
	}
}

void Scene::write_params_to_application_struct(ApplicationUboDataStructure& app_ubo)
{
	app_ubo.scene_params.x = m_tile_x_count;
	app_ubo.scene_params.y = m_seed;
	app_ubo.scene_params.z = glm::floatBitsToUint(m_tile_size);//store the 32 bits of  float in an unsigned int (UintBitsToFloat to be called in shader)
	app_ubo.scene_params.w = glm::floatBitsToUint(m_y_offset);
}


void Scene::render_scene()
{
	m_scene_vao.use_vao();
	m_shader_scene->use_shader_program();
	//The tessellation shader will generate one subdivised quad per "GL_PATCHES", thus we have to set it to the number of tiles
	glDrawArrays(GL_PATCHES, 0, m_tile_x_count * m_tile_x_count);
	glFlush();
	m_shader_background->use_shader_program();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glFlush();

}

void Scene::gui(ApplicationUboDataStructure& app_ubo)
{
	if (ImGui::TreeNode("Scene"))
	{
		ImGui::SliderInt("Terrain seed", &(m_seed), 1, 1000000);
		ImGui::SliderInt("Tile tessellation level", &(m_tessellation_base_level), 1, 64);
		ImGui::SliderFloat("Terrain offset", &(m_y_offset), -10.0f, 10.0f);

		m_tile_x_count = glm::max(1, m_tile_x_count);
		m_tile_size = glm::max(0.01f, m_tile_size);
		m_tessellation_base_level = glm::clamp(m_tessellation_base_level,1,64);

		ImGui::Text(("Scene triangle count: " + std::to_string(m_tile_x_count * m_tile_x_count*(2* m_tessellation_base_level* m_tessellation_base_level))).c_str());
		ImGui::TreePop();
	}
}