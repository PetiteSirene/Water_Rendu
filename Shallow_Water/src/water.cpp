#include "water.hpp"

Water::Water()
{
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
    glDrawArrays(GL_PATCHES, 0, 24 * 24);
    glFlush();

}

void Water::InitializeTextures(int size)
{
    //physics part
    std::vector<glm::vec2> physicData(size * size, glm::vec2(0.0f, 0.0f));
    glBindTexture(GL_TEXTURE_2D, texturePhysics);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, size, size, 0, GL_RG, GL_FLOAT, physicData.data());
    glBindTexture(GL_TEXTURE_2D, TEXTURE_SLOT_WATER_PHYSICS);

    //normals part
    std::vector<glm::vec4> normalData(size * size, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    glBindTexture(GL_TEXTURE_2D, textureNormals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, normalData.data());
    glBindTexture(GL_TEXTURE_2D, TEXTURE_SLOT_WATER_NORMALS);
}

void Water::BindPhysicsTexture(GLuint unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texturePhysics);
}

void Water::BindNormalsTexture(GLuint unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureNormals);
}