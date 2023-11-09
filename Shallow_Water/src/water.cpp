#include "water.hpp"

Water::Water()
{
    m_tesselation_level = 1;
    refractive_index = 1.333f;
    absorbance = 0.01f;//fraction per meter
    reflection_ratio = 0.5f;
    color = vec4(0.0f, 0.0f, 1.0f,0.0f);
    m_shader_water = new ShaderGLSL("water_shader");
    simulate_shader = new ShaderGLSL("simulation_shader");
    normal_shader = new ShaderGLSL("normal_shader");
    copy_shader = new ShaderGLSL("copy_shader");
    celerity = 5.0f;
    damping = 0.003f;
    simulation_resolution = 1024;
    delta_t_sec = 0.002f;
    disturb_height = 1.0;
    InitializeTextures(simulation_resolution);
}

void Water::load_shaders(std::string base_path)
{
    m_shader_water->add_shader(GL_VERTEX_SHADER, base_path, "shaders/water_vs.glsl");
    m_shader_water->add_shader(GL_TESS_EVALUATION_SHADER, base_path, "shaders/water_tes.glsl");
    m_shader_water->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/water_fs.glsl");
    m_shader_water->compile_and_link_to_program();
    ContextHelper::add_shader_to_hot_reload(m_shader_water);

    
    simulate_shader->add_shader(GL_COMPUTE_SHADER, base_path, "shaders/water_simulate_cs.glsl");
    simulate_shader->compile_and_link_to_program();
    ContextHelper::add_shader_to_hot_reload(simulate_shader);

    copy_shader->add_shader(GL_COMPUTE_SHADER, base_path, "shaders/water_copy_cs.glsl");
    copy_shader->compile_and_link_to_program();
    ContextHelper::add_shader_to_hot_reload(copy_shader);
    
    normal_shader->add_shader(GL_COMPUTE_SHADER, base_path, "shaders/water_normal_cs.glsl");
    normal_shader->compile_and_link_to_program();
    ContextHelper::add_shader_to_hot_reload(normal_shader);

}

void Water::render_water()
{
    m_shader_water->use_shader_program();
    //The tessellation shader will generate one subdivised quad per "GL_PATCHES", thus we have to set it to the number of tiles
    glDrawArrays(GL_PATCHES, 0, simulation_resolution * simulation_resolution);
    glFlush();

}

void Water::InitializeTextures(int size)
{
    //Can be only called once !
   
    //physics part
    std::vector<glm::vec4> physicData(size * size, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    /*
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            physicData[size*(size/2+i) + (size/2 + j)] = vec4(5.0f, 5.0f, 5.0f, 5.0f);
        }

    }*/
    glCreateTextures(GL_TEXTURE_2D, 1, &texturePhysics);
    glTextureParameteri(texturePhysics, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texturePhysics, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureStorage2D(texturePhysics, 1,GL_RGBA32F, size, size);//Immutable size
    glTextureSubImage2D(texturePhysics, 0, 0, 0, size, size, GL_RGBA, GL_FLOAT, physicData.data());
    glBindTextureUnit(TEXTURE_SLOT_WATER_PHYSICS, texturePhysics);
    glBindImageTexture(TEXTURE_SLOT_WATER_PHYSICS, texturePhysics, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
    //normals part
    std::vector<glm::vec4> normalData(size * size, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    glCreateTextures(GL_TEXTURE_2D, 1, &textureNormals);
    glTextureStorage2D(textureNormals, 1, GL_RGBA32F, size, size);//Immutable size
    glTextureSubImage2D(textureNormals, 0, 0, 0, size, size, GL_RGBA, GL_FLOAT, normalData.data());
    glBindTextureUnit(TEXTURE_SLOT_WATER_NORMALS, textureNormals);
    glBindImageTexture(TEXTURE_SLOT_WATER_NORMALS, textureNormals, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

}

void Water::simulate_water()
{
    const uvec2 work_group_size = uvec2(8, 8);//MUST MATCH COMPUTE SHADER
    const uvec2 dispatch_count = uvec2((simulation_resolution - 1u), (simulation_resolution - 1u)) / work_group_size + uvec2(1u, 1u);

    for (int i = (int)((ContextHelper::time_from_start_s-ContextHelper::time_frame_s)/ delta_t_sec); i < (int)(ContextHelper::time_from_start_s / delta_t_sec); i++)
    {
        simulate_shader->use_shader_program();
        glDispatchCompute(dispatch_count.x, dispatch_count.y, 1);//Dispatch that covers screen 
        glFlush();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);//Sync barrier to ensure CS finished (since it is writing to an Image)
        
        copy_shader->use_shader_program();
        glDispatchCompute(dispatch_count.x, dispatch_count.y, 1);//Dispatch that covers screen 
        glFlush();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);//Sync barrier to ensure CS finished (since it is writing to an Image)
    }
    normal_shader->use_shader_program();
    glDispatchCompute(dispatch_count.x, dispatch_count.y, 1);
    glFlush();
    glFinish();



}


int Water::GetSimulationResolution()
{
    return simulation_resolution;
}

void Water::write_params_to_application_struct(ApplicationUboDataStructure& app_ubo)
{
    float map_length = app_ubo.scene_params.x * uintBitsToFloat(app_ubo.scene_params.z);
    //Compute texelcoords for water perturbation
    double mouse_x, mouse_y;
    glfwGetCursorPos(ContextHelper::window, &mouse_x, &mouse_y);
    vec4 ndc_mouse = vec4(mouse_x / ContextHelper::resolution.x * 2.0f - 1.0f, 1.0f-mouse_y / ContextHelper::resolution.y * 2.0f ,1.0f,1.0f);
    vec4 mouse_ws = app_ubo.inv_w_v_p * ndc_mouse;
    mouse_ws /= mouse_ws.w;
    vec3 dir = vec3(mouse_ws) - vec3(app_ubo.cam_pos);//not normalized
    dir /= dir.y;//dir.y = 1.0f now 
    float dy = 0.0f - app_ubo.cam_pos.y;
    dir *= dy;//dir.y = dy !
    vec3 pos_intersection = vec3(app_ubo.cam_pos) + dir;
    pos_intersection += vec3(map_length * 0.5);
    pos_intersection /= map_length;//in [0.0,1.0]
    app_ubo.resolution.z = (int)glm::packUnorm2x16(vec2(pos_intersection.x, pos_intersection.z));


    app_ubo.resolution.w = simulation_resolution;
    app_ubo.water_sim_params.x = celerity;
    app_ubo.water_sim_params.y = damping;
    app_ubo.water_sim_params.z = delta_t_sec;
    app_ubo.water_sim_params.w = map_length / simulation_resolution;
    app_ubo.water_rendering_params.x = refractive_index;
    app_ubo.water_rendering_params.y = absorbance;
    app_ubo.water_rendering_params.z = reflection_ratio;
    app_ubo.water_rendering_params.w = glfwGetMouseButton(ContextHelper::window, GLFW_MOUSE_BUTTON_LEFT)==1 ? 1.0f : 0.0f;//is clicked
    app_ubo.water_aborption_color = vec4(color.r, color.g, color.b, disturb_height);
}

void Water::flush_tessellation_levels()
{
    GLfloat levels[] = { (float)m_tesselation_level, (float)m_tesselation_level, (float)m_tesselation_level, (float)m_tesselation_level };
    glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);
    glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, levels);
}

void Water::gui(ApplicationUboDataStructure& app_ubo)
{
    if (ImGui::TreeNode("Water")) {
        if (ImGui::Button("Reset water"))
            InitializeTextures(simulation_resolution);
        ImGui::SliderFloat("Disturb height", &disturb_height,-10.0f,10.0f);
        ImGui::Text(("A factor = " + std::to_string(celerity* celerity* delta_t_sec* delta_t_sec/(app_ubo.water_sim_params.w* app_ubo.water_sim_params.w))).c_str());
        ImGui::ColorEdit3("Absorbing color", &(color.x));
        ImGui::SliderInt("Water tessellation level", &(m_tesselation_level), 1, 8);
        ImGui::SliderFloat("Celerity", &celerity, 0.1f, 10.0f);
        ImGui::SliderFloat("damping", &damping, 0.0f, 0.1f);
        ImGui::SliderFloat("refractive index", &refractive_index, 1.0f, 4.0f);
        ImGui::SliderFloat("absorbance", &absorbance, 0.0f, 1.0f);
        ImGui::SliderFloat("reflection ratio", &reflection_ratio, 0.0f, 1.0f);
        ImGui::TreePop();
    }
}
