#define GLFW_INCLUDE_NONE
#define TINYOBJLOADER_C_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_LEFT_HANDED // because of OpenGL NDC
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <thread>
#include <tiny_obj_loader_c.h>
#include <stb_image_write.h>
#include "helpers/helpers_common.hpp"
#include "buffer_structures.hpp"

#include "scene.hpp"
#include "water.hpp"

#define FOLDER_ROOT "../"


int main(int argc, char* argv[]) {

	ContextHelper::init_context_all(1440, 900, "Shallow Water",4);//No MSAA, because of Deferred shading
	ContextHelper::print_opengl_info();

	//Projection matrix
	ProjectionMatrix proj;
	proj.set_viewport_resolution(ContextHelper::resolution);
	proj.set_perspective(70.0f, 0.1f, 500.0f);//maybe to adjust to scene

	float size_y = 1.f;
	ProjectionMatrix ortho_proj;
	ortho_proj.set_viewport_resolution(ContextHelper::resolution);
	ortho_proj.set_ortho_centered(48.0f,0.1f,500.0f);
	//Top-Down WorldView matrix
	FreeFlyCamera topdown_cam;
	topdown_cam.set_camera(vec3(0.f, 35.f, 0.f), 0.f, -90.f);

	//WorldView matrix
	FreeFlyCamera cam; // Maybe this class will be modified to have a "walk" mode (forced just above the ground)
	cam.set_camera(vec3(-35.0f,30.0f,-30.0f),45.0f,-20.0f);
	cam.set_params(0.1f,0.5f,50.0f);


	Scene scene;
	scene.load_shaders(FOLDER_ROOT);
	scene.create_material_texture_array(FOLDER_ROOT,"rock");

	Water water;
	water.load_shaders(FOLDER_ROOT);

	Framebuffer depth_buffer;
	//We create a framebuffer that contain only the z buffer
	depth_buffer.create_framebuffer(0,{}, true);
	depth_buffer.update_size(ContextHelper::resolution);


	//UBO init
	ApplicationUboDataStructure app_ubo_data;
	GPUBuffer application_ubo;
	application_ubo.allocate(sizeof(ApplicationUboDataStructure));
	application_ubo.set_target_and_slot(GL_UNIFORM_BUFFER, UBO_APPLICATION_BINDING);

	app_ubo_data.sun_light = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	app_ubo_data.resolution.x = ContextHelper::resolution.x;
	app_ubo_data.resolution.y = ContextHelper::resolution.y;
	app_ubo_data.resolution.z = 0;//Normal display

	ShaderGLSL* compute_shader = new ShaderGLSL("compute_shader");
	compute_shader->add_shader(GL_COMPUTE_SHADER, FOLDER_ROOT, "shaders/water_cs.glsl");
	compute_shader->compile_and_link_to_program();
	ContextHelper::add_shader_to_hot_reload(compute_shader);

	// At the end of the compute shader, the pixel is white if it is below the water level.
	Texture2D isWaterTexture;
	isWaterTexture.set_format_params({ GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE,4 });
	isWaterTexture.set_filtering_params();
	isWaterTexture.create_empty(ContextHelper::resolution);
	isWaterTexture.set_slot(1);
	isWaterTexture.bind_to_image(GL_WRITE_ONLY);




	//OpenglFlags
	bool draw_wireframe = false;
	glClearColor(0.0f,0.0f, 0.0f,glm::intBitsToFloat(0)); // background is black
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glPatchParameteri(GL_PATCH_VERTICES, 1);//1 Patch per ground tile

	while (ContextHelper::should_not_close_window()) //main/render loop
	{
		ContextHelper::begin_frame();
		if (ContextHelper::window_resized)
		{
			glViewport(0, 0, ContextHelper::resolution.x, ContextHelper::resolution.y);
			depth_buffer.update_size(ContextHelper::resolution);
			isWaterTexture.re_create_empty(ContextHelper::resolution);
			isWaterTexture.bind_to_image(GL_WRITE_ONLY);
		}
		scene.flush_tessellation_levels();

		proj.set_viewport_resolution(ContextHelper::resolution);
		cam.flush();
		
		glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer.m_framebuffer_id);

		//Flush Application UBO
		app_ubo_data.proj = ortho_proj.m_proj;
		app_ubo_data.inv_proj = glm::inverse(ortho_proj.m_proj);
		app_ubo_data.w_v = topdown_cam.m_w_v;
		app_ubo_data.w_v_p = ortho_proj.m_proj * topdown_cam.m_w_v;
		app_ubo_data.inv_w_v_p = glm::inverse(app_ubo_data.w_v_p);
		app_ubo_data.cam_pos = vec4(topdown_cam.m_pos, ContextHelper::time_from_start_s);
		app_ubo_data.resolution.x = ContextHelper::resolution.x;
		app_ubo_data.resolution.y = ContextHelper::resolution.y;
		scene.write_params_to_application_struct(app_ubo_data);

		application_ubo.write_to_gpu(&app_ubo_data);


		if (draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


		glClear(GL_DEPTH_BUFFER_BIT);//clear Frambuffer channel + Z-buffer 
		scene.render_scene();//Render the scene without water
		glFinish();//Force wait for GPU to finish jobs, since the post_process shader will read from rendered textures
		water.render_water();
		glFinish();

		const uvec2 work_group_size = uvec2(8, 8);//MUST MATCH COMPUTE SHADER

		const uvec2 dispatch_count = uvec2((ContextHelper::resolution.x + (work_group_size.x - 1)) / work_group_size.x,
			(ContextHelper::resolution.y + (work_group_size.y - 1)) / work_group_size.y);

		compute_shader->use_shader_program();//Compute shader for SSAO + depth blur
		glDispatchCompute(dispatch_count.x, dispatch_count.y, 1);//Dispatch that covers screen 
		glFlush();
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);//Sync barrier to ensure CS finished (since it is writing to an Image)

		app_ubo_data.proj = proj.m_proj;
		app_ubo_data.inv_proj = glm::inverse(proj.m_proj);
		app_ubo_data.w_v = cam.m_w_v;
		app_ubo_data.w_v_p = proj.m_proj * cam.m_w_v;
		app_ubo_data.inv_w_v_p = glm::inverse(app_ubo_data.w_v_p);
		app_ubo_data.cam_pos = vec4(cam.m_pos, ContextHelper::time_from_start_s);
		app_ubo_data.resolution.x = ContextHelper::resolution.x;
		app_ubo_data.resolution.y = ContextHelper::resolution.y;
		scene.write_params_to_application_struct(app_ubo_data);
		application_ubo.write_to_gpu(&app_ubo_data);

		glBindFramebuffer(GL_FRAMEBUFFER,0);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear Frambuffer channel + Z-buffer 
		scene.render_scene();//Render the scene without water
		glFinish();//Force wait for GPU to finish jobs, since the post_process shader will render
		if (draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//ImGui interface starts here
		ImGui::Begin("Parameters");
		static float theta_light = 30.0f;
		static float phi_light = 20.0f;
		float ct = glm::cos(glm::radians(theta_light));
		float st = glm::sin(glm::radians(theta_light));
		float cp = glm::cos(glm::radians(phi_light));
		float sp = glm::sin(glm::radians(phi_light));
		app_ubo_data.sun_light.x = cp * ct;
		app_ubo_data.sun_light.y = sp;
		app_ubo_data.sun_light.z = cp * st;

		if (ImGui::TreeNode("General"))
		{
			static float font_scale = 1.0f;
			ImGui::SetWindowFontScale(font_scale);
			ImGui::SliderFloat("text scale", &font_scale, 0.5f, 4.0f);
			
			ImGui::Text(("Camera position: " + std::to_string(cam.m_pos.x) + " " + std::to_string(cam.m_pos.y) + " " + std::to_string(cam.m_pos.z) + " ").c_str());
			ImGui::Text(("Camera direction: " + std::to_string(cam.m_w_v[0].z) + " " + std::to_string(cam.m_w_v[1].z) + " " + std::to_string(cam.m_w_v[2].z) + " ").c_str());
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Debug"))
		{
			ImGui::Checkbox("Draw wireframe", &draw_wireframe);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Sun"))
		{
		ImGui::SliderFloat("Sun light theta", &(theta_light),0.0f,180.0f);
		ImGui::SliderFloat("Sun light phi", &(phi_light), 0.0f, 179.9f);
		ImGui::Text(("Sun light direction: " + std::to_string(app_ubo_data.sun_light.x) + " " + std::to_string(app_ubo_data.sun_light.y) + " " + std::to_string(app_ubo_data.sun_light.z) + " ").c_str());
		ImGui::SliderFloat("Sun light intensity", &app_ubo_data.sun_light.w, 0.0f, 2.0f);
		ImGui::TreePop();
		}
		scene.gui(app_ubo_data);

		ImGui::End();

		ContextHelper::end_frame();//glfwSwapBuffers [hookpoint for profiler/debugger]
	}
	ContextHelper::destroy_context_all();

	return EXIT_SUCCESS;
}
