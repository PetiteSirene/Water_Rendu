#ifndef WATER_CLASS
#define WATER_CLASS

#include "buffer_structures.hpp"
#include "helpers/helpers_common.hpp"
#include <GLFW/glfw3.h>
#include <vector>

class Water {
public:
    Water();
    void load_shaders(std::string base_path);
    void render_water();

    //creation des textures
    void InitializeTextures(int size);
    int GetSimulationResolution();

    void BindPhysicsTexture(GLuint unit);
    void BindNormalsTexture(GLuint unit);

    void write_params_to_application_struct(ApplicationUboDataStructure& app_ubo);
    void flush_tessellation_levels();

    void gui(ApplicationUboDataStructure& app_ubo);

private:
    ShaderGLSL* m_shader_water;

    int simulation_resolution;
    int m_tesselation_level;

    // Ajout des textures
    GLuint texturePhysics; //2 canaux: hauteur, vitesse
    GLuint textureNormals; //4 canaux : 3 pour la normale, 1 unused

    float refractive_index;
    float absorbance;
    vec3 color;
};

#endif
