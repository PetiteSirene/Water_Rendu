#version 460
#define UBO_APPLICATION_BINDING 0
layout (local_size_x = 8, local_size_y = 8,local_size_z = 1) in;

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

layout(r32f,binding = 0) uniform readonly restrict image2D tex_altitude_ws;
layout(rgba32f,binding = 2) uniform image2D water_physic;//output image

void main(){

	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

	if (coord.x>=resolution.w || coord.y>=resolution.w)
        return; //Do not process out of simulation texture

    float dt = water_sim_params.z;
    float h = water_sim_params.w;
    float c = water_sim_params.x;

    //Reads from z,w (height, speed)
    //Writes to x,y (height, speed)

    float a = c*c*dt*dt/(h*h);
    float p_prev = imageLoad(water_physic,coord).w;
    float p = imageLoad(water_physic, coord).z;
    float p1 = imageLoad(water_physic, coord + ivec2(0,1)).z;
    float p2 = imageLoad(water_physic, coord + ivec2(1,0)).z;
    float p3 = imageLoad(water_physic, coord + ivec2(0,-1)).z;
    float p4 = imageLoad(water_physic, coord + ivec2(-1,0)).z;

    float p_new = a * (p1 + p2 + p3 + p4 - 4.0*p) + (1.0 -water_sim_params.y*dt*80.0)*(p - p_prev) + p;

    if (imageLoad(tex_altitude_ws, coord).x<0.0)
        imageStore(water_physic, coord, vec4(p_new,p,0.0,0.0));
    //else
    //    imageStore(water_physic, coord, vec4(20.0,20.0,0.0,0.0));
}