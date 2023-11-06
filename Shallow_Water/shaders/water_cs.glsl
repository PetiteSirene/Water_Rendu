#version 460
#define UBO_APPLICATION_BINDING 0
layout (local_size_x = 8, local_size_y = 8,local_size_z = 1) in;

layout(binding = UBO_APPLICATION_BINDING, std140) uniform UBO_APPLICATION
{
  //General
    mat4 proj; //projection matrix (view to eye)
    mat4 inv_proj; //inverse projection matrix (eye to view)
    mat4 w_v; //world to view matrix
    mat4 w_v_p; //world to eye matrix
    mat4 inv_w_v_p; //eye to world matrix
    vec4 cam_pos;//camera position in world space, .w: time
    ivec4 resolution; //.x:resolution.x, .y:resolution.y, .z: debug_mode, .w:unused
    //Light
    vec4 sun_light;//.xyz: direction, .w:intensity
    //Scene
    uvec4 scene_params; //.x:tile_count, .y:seed, .z:tile_size[FLOAT], .w:map_offset[FLOAT]
    vec4 water_params; //.x:resolution[INT], .y:absorbance, .z:water_tile_size[FLOAT]
};

layout(binding = 0) uniform sampler2D depth_buffer;//G-Buffer,read from, in [0.0,1.0] !

layout(rg32f,binding = 2) uniform image2D water_physic;//output image

float deltaT = intBitsToFloat(resolution.w);
float h = water_params.z;
float c = water_params.w;

float read_depth(vec2 center)
{
    float ndc_depth =  2.0*textureLod(depth_buffer,center,0.0).x-1.0;
    return ndc_depth;
}

vec3 WorldPosFromCoord(vec2 n_coords){

	float depth = read_depth(n_coords);

	vec4 clipSpacePosition = vec4(n_coords * 2.0 - 1.0, depth, 1.0);
  //Skipping perspective division because comparing to zero ?
	return (inv_w_v_p * clipSpacePosition).xyz;
	}

void compute_speed(ivec2 coord){
    float a = pow(c,2)*pow(deltaT,2)/pow(h,2);
    float Vt = imageLoad(water_physic,coord).g;
    float p = imageLoad(water_physic, coord).r;
    float p1 = imageLoad(water_physic, coord + ivec2(0,1)).r;
    float p2 = imageLoad(water_physic, coord + ivec2(1,0)).r;
    float p3 = imageLoad(water_physic, coord + ivec2(0,-1)).r;
    float p4 = imageLoad(water_physic, coord + ivec2(-1,0)).r;
    float newVt = a * (p1 + p2 + p3 + p4) + (2 - 4*a) * p - deltaT * Vt;
    imageStore(water_physic, coord, vec4(1.0,1.0,0,0));
}

void main(){

    vec4 white = vec4(1.0,1.0,1.0,1.0);
    vec4 black = vec4(0.0,0.0,0.0,1.0);

	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

	if (coord.x>=resolution.x || coord.y>=resolution.y)
        return; //Do not process out of screen

	vec2 n_coords = (vec2(coord) + 0.5) / resolution.xy;
       
    compute_speed(coord);
    
}