#version 460
#define UBO_APPLICATION_BINDING 0
layout(quads, equal_spacing, ccw) in;
in vec2 ijCoords[];

//matches buffer_structures.hpp
layout(binding = UBO_APPLICATION_BINDING, std140) uniform UBO_APPLICATION
{
	mat4 proj; //projection matrix (view to eye)
	mat4 inv_proj; //inverse projection matrix (eye to view)
	mat4 w_v; //world to view matrix
	mat4 w_v_p; //world to eye matrix
	mat4 inv_w_v_p; //eye to world matrix
	vec4 cam_pos;//camera position in world space, .w: time
	ivec4 resolution; //.x:resolution.x, .y:resolution.y, .z: packed_mouse_coords, .w: sim_resolution
	vec4 sun_light;//.xyz: direction, .w:intensity
	uvec4 scene_params; //.x:tile_count, .y:seed, .z:tile_size[FLOAT], .w:map_offset[FLOAT]
	vec4 water_sim_params; //.x:celerity, .y: damping, .z:delta_time, .w: sim_tile_length
	vec4 water_rendering_params; // .x:refractive_index, .y:absorbance, .z:reflection_ratio, w.:mouse click(1.0 if clicked, 0.0 if not)
	vec4 water_absorption_color;// .xyz absorption color .w = disturb amplitude
	vec4 water_raymarching_params; // .x step, .y: min_dist, .z:max_dist, [UINT].w: max_iter
};

layout(binding = 2) uniform sampler2D physicsTexture;
out vec3 pos;
out vec2 tex_coords;//in [0.0,1.0]
float tile_size = water_sim_params.w;

void main() 
{
    tex_coords = (ijCoords[0] + gl_TessCoord.xy)/float(resolution.w);
    pos.xz = tile_size*(ijCoords[0]+gl_TessCoord.xy) - scene_params.x * uintBitsToFloat(scene_params.z) * 0.5;
    pos.y = textureLod(physicsTexture,tex_coords,0.0).x;
    gl_Position = w_v_p * vec4(pos,1.0);
}