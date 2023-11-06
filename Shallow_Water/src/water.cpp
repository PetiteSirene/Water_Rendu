#include "water.hpp"

Water::Water()
{
    m_tesselation_level = 1;
    refractive_index = 1.333f;
    absorbance = 100.0f;
    color = vec3(0.0f, 0.0f, 1.0f);
    m_shader_water = new ShaderGLSL("water_shader");

    simulation_resolution = 512;

    InitializeTextures(simulation_resolution);
}

void Water::load_shaders(std::string base_path)
{
    m_shader_water->add_shader(GL_VERTEX_SHADER, base_path, "shaders/water_vs.glsl");
    m_shader_water->add_shader(GL_TESS_EVALUATION_SHADER, base_path, "shaders/water_tes.glsl");
    m_shader_water->add_shader(GL_FRAGMENT_SHADER, base_path, "shaders/water_fs.glsl");
    m_shader_water->compile_and_link_to_program();
    ContextHelper::add_shader_to_hot_reload(m_shader_water);
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
    std::vector<glm::vec2> physicData(size * size, glm::vec2(0.0f, 0.0f));
    glCreateTextures(GL_TEXTURE_2D, 1, &texturePhysics);
    //glBindTexture(GL_TEXTURE_2D, texturePhysics);
    glTextureParameteri(texturePhysics, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texturePhysics, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureStorage2D(texturePhysics, 1,GL_RG32F, size, size);//Immutable size
    glTextureSubImage2D(texturePhysics, 0, 0, 0, size, size, GL_RG, GL_FLOAT, physicData.data());
    glBindTextureUnit(TEXTURE_SLOT_WATER_PHYSICS, texturePhysics);
    glBindImageTexture(TEXTURE_SLOT_WATER_PHYSICS, texturePhysics, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RG32F);
    //normals part
    std::vector<glm::vec4> normalData(size * size, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    glCreateTextures(GL_TEXTURE_2D, 1, &textureNormals);
    //glBindTexture(GL_TEXTURE_2D, textureNormals);
    glTextureStorage2D(textureNormals, 1, GL_RGBA32F, size, size);//Immutable size
    glTextureSubImage2D(textureNormals, 0, 0, 0, size, size, GL_RGBA, GL_FLOAT, normalData.data());
    glBindTextureUnit(TEXTURE_SLOT_WATER_NORMALS, textureNormals);
    glBindImageTexture(TEXTURE_SLOT_WATER_NORMALS, textureNormals, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

}

int Water::GetSimulationResolution()
{
    return simulation_resolution;
}

//No need I guess
void Water::BindPhysicsTexture(GLuint unit)
{
    glBindTextureUnit(unit, texturePhysics);
}

void Water::BindNormalsTexture(GLuint unit)
{
    glBindTextureUnit(unit, textureNormals);
}

void Water::write_params_to_application_struct(ApplicationUboDataStructure& app_ubo)
{
    app_ubo.water_params.x = simulation_resolution;
    app_ubo.water_params.y = absorbance;
    app_ubo.water_params.z = 24 * 4.0f / simulation_resolution;
    app_ubo.water_params.w = 2.0f;
}

void Water::flush_tessellation_levels()
{
    //static int tess = -1;

    //if (tess != m_tessellation_base_level)
    {
        GLfloat levels[] = { m_tesselation_level, m_tesselation_level, m_tesselation_level, m_tesselation_level };
        glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);
        glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, levels);
        //tess = m_tessellation_base_level;
    }
}

void Water::gui(ApplicationUboDataStructure& app_ubo)
{
    if (ImGui::TreeNode("Water")) {
        ImGui::SliderInt("Water tessellation level", &(m_tesselation_level), 1, 8);
        ImGui::TreePop();
    }
}
