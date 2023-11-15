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
    void simulate_water();
    //creation des textures
    void InitializeTextures(int size);
    int GetSimulationResolution();

    void write_params_to_application_struct(ApplicationUboDataStructure& app_ubo);
    void flush_tessellation_levels();

    void gui(ApplicationUboDataStructure& app_ubo);

private:
    ShaderGLSL* m_shader_water;
    ShaderGLSL* simulate_shader;
    ShaderGLSL* copy_shader;
    ShaderGLSL* normal_shader;

    int simulation_resolution;
    int m_tesselation_level;

    // Ajout des textures
    GLuint texturePhysics; //2 canaux: hauteur, vitesse
    GLuint textureNormals; //4 canaux : 3 pour la normale, 1 unused

    float delta_t_sec;
    float celerity;
    float damping;
    float refractive_index;
    float absorbance;
    float reflection_ratio;
    float disturb_height;
    vec3 color;
    float raymarch_step;
    int raymarch_max_iter;
    float raymarch_min_dist;
    float raymarch_max_dist;
};

#endif
