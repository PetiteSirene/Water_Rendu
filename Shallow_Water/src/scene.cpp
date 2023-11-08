#include "scene.hpp"

Scene::Scene()
{
	m_tessellation_base_level = 16;
	m_tile_size = 4.0f;
	m_tile_x_count = 24;
	m_seed = 0;
	m_y_offset = 0.0f;
	m_shader_scene = new ShaderGLSL("scene_shader");
	m_shader_scene_depth = new ShaderGLSL("scene_depth_shader");
	m_shader_background = new ShaderGLSL("background_shader");
	//We create a framebuffer that contain only the altitude (in worlds space). There is no need of Z-buffer since this is a heigh field seen from top => 1 fragment/ pixel
	fbo_scene_depth.create_framebuffer(1, { GL_R32F,GL_RED,GL_FLOAT,1 }, false);
	flush_tessellation_levels();
}

void Scene::load_shaders(std::string base_path)
{
	m_shader_scene->add_shader(GL_VERTEX_SHADER, base_path, "shaders/scene_vs.glsl");
	m_shader_scene->add_shader(GL_TESS_EVALUATION_SHADER, base_path, "shaders/scene_tes.glsl");
	m_shader_scene->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/scene_fs.glsl");//write color + depth
	m_shader_scene->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(m_shader_scene);

	m_shader_scene_depth->add_shader(GL_VERTEX_SHADER, base_path, "shaders/scene_vs.glsl");
	m_shader_scene_depth->add_shader(GL_TESS_EVALUATION_SHADER, base_path, "shaders/scene_tes.glsl");
	m_shader_scene_depth->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/scene_depth_fs.glsl");// only depth
	m_shader_scene_depth->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(m_shader_scene_depth);

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
	//static int tess = -1;

	//if (tess != m_tessellation_base_level)
	{
		GLfloat levels[] = { m_tessellation_base_level, m_tessellation_base_level, m_tessellation_base_level, m_tessellation_base_level };
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, levels);
		//tess = m_tessellation_base_level;
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

//To be called on:
// - terrain offset change
// - terrain seed change
// - simulation resolution change
void Scene::render_scene_depth(GPUBuffer& application_ubo,int simulation_resolution)
{
	//Set camera parameters

	//Top down matrix
	//looking in  -y dir
	ProjectionMatrix ortho_proj;
	ortho_proj.set_viewport_resolution(vec2(simulation_resolution, simulation_resolution));
	ortho_proj.set_ortho_centered(m_tile_size * m_tile_x_count * 0.5f, 0.1f, 500.0f);

	glm::vec3 ortho_top_pos = glm::vec3(0.0, 100.0f, 0.0);
	glm::mat4 ortho_top_w_v = glm::mat4(0.0f);
	ortho_top_w_v[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);//column1
	ortho_top_w_v[1] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);//column2
	ortho_top_w_v[2] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);//column3
	ortho_top_w_v[3] = glm::vec4(ortho_top_pos.x, -ortho_top_pos.z, ortho_top_pos.y, 1.0f);//column4

	ApplicationUboDataStructure app_ubo_data_depth;

	app_ubo_data_depth.proj = ortho_proj.m_proj;//
	app_ubo_data_depth.inv_proj = glm::inverse(app_ubo_data_depth.proj);
	app_ubo_data_depth.w_v = ortho_top_w_v;//
	app_ubo_data_depth.w_v_p = app_ubo_data_depth.proj * app_ubo_data_depth.w_v;
	app_ubo_data_depth.inv_w_v_p = glm::inverse(app_ubo_data_depth.w_v_p);
	app_ubo_data_depth.cam_pos = vec4(ortho_top_pos, ContextHelper::time_from_start_s);//
	write_params_to_application_struct(app_ubo_data_depth);
	application_ubo.write_to_gpu(&app_ubo_data_depth);

	fbo_scene_depth.update_size(glm::uvec2(simulation_resolution, simulation_resolution));
	fbo_scene_depth.bind_framebuffer();
	glClear(GL_COLOR_BUFFER_BIT); //not useful actually
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, simulation_resolution, simulation_resolution);
	m_scene_vao.use_vao();
	m_shader_scene_depth->use_shader_program();
	glDrawArrays(GL_PATCHES, 0, m_tile_x_count * m_tile_x_count);
	//No need of background
	glFinish();

	//revert opengl States for main renderings
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, ContextHelper::resolution.x, ContextHelper::resolution.y);
}

//return true if depth needs to be recomputed
bool Scene::gui(ApplicationUboDataStructure& app_ubo)
{
	static int prev_seed = m_seed;
	static int prev_tessellation_base_level = m_tessellation_base_level;
	static float prev_y_offset = m_y_offset;
	bool state_changed = false;
	if (ImGui::TreeNode("Scene"))
	{
		ImGui::SliderInt("Terrain seed", &(m_seed), 1, 1000000);
		if (prev_seed != m_seed)
		{
			prev_seed = m_seed;
			state_changed = true;
		}
		ImGui::SliderInt("Tile tessellation level", &(m_tessellation_base_level), 1, 64);
		if (prev_tessellation_base_level != m_tessellation_base_level)
		{
			prev_tessellation_base_level = m_tessellation_base_level;
			state_changed = true;
		}
		ImGui::SliderFloat("Terrain offset", &(m_y_offset), -10.0f, 10.0f);
		if (prev_y_offset != m_y_offset)
		{
			prev_y_offset = m_y_offset;
			state_changed = true;
		}

		m_tile_x_count = glm::max(1, m_tile_x_count);
		m_tile_size = glm::max(0.01f, m_tile_size);
		m_tessellation_base_level = glm::clamp(m_tessellation_base_level,1,64);

		ImGui::Text(("Scene triangle count: " + std::to_string(m_tile_x_count * m_tile_x_count*(2* m_tessellation_base_level* m_tessellation_base_level))).c_str());
		ImGui::TreePop();
	}
	return state_changed;
}