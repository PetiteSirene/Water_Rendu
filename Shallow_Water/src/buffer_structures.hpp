#ifndef BUFFER_STRUCTURES
#define BUFFER_STRUCTURES

#include <glm/glm.hpp>

#define uint uint32_t // for compatibility with GLSL
using namespace glm;

//structures for uniform buffers (UBOs) require proper 16 byte alignment (std140 layout)
//structures for shader storage buffers (SSBOs) require only 4 byte alignment (std430 layout)
//more at https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout

//Texture channels ID:
#define TEXTURE_SLOT_ALTITUDE_WS 0 // from FBO
#define TEXTURE_SLOT_WATER_PHYSICS 2
#define TEXTURE_SLOT_WATER_NORMALS 3
#define TEXTURE_SLOT_ALBEDO_GROUND 4
#define TEXTURE_SLOT_NORMAL_GROUND 5
#define TEXTURE_SLOT_ROUGHNESS_GROUND 6
#define TEXTURE_SLOT_AMBIENT_OCCLUSION_GROUND 7
#define TEXTURE_SLOT_DISPLACEMENT_GROUND 8

//GLSL: layout(binding = 0, std140) uniform UBO_APPLICATION
#define UBO_APPLICATION_BINDING 0
struct ApplicationUboDataStructure
{
	mat4 proj; //projection matrix (view to eye)
	mat4 inv_proj; //inverse projection matrix (eye to view)
	mat4 w_v; //world to view matrix
	mat4 w_v_p; //world to eye matrix
	mat4 inv_w_v_p; //eye to world matrix
	vec4 cam_pos;//camera position in world space, .w: time
	ivec4 resolution; //.x:resolution.x, .y:resolution.y, .z: debug_mode, .w: sim_resolution
	vec4 sun_light;//.xyz: direction, .w:intensity
	uvec4 scene_params; //.x:tile_count, .y:seed, .z:tile_size[FLOAT], .w:map_offset[FLOAT]
	vec4 water_sim_params; //.x:celerity, .y: damping, .z:delta_time, .w: sim_tile_length
	vec4 water_rendering_params;
	vec4 water_aborption_color;
};

#endif

