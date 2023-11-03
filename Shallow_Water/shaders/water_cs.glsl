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
};

layout(binding = 0) uniform sampler2D depth_buffer;//G-Buffer,read from, in [0.0,1.0] !

layout(binding = 2, rgba8) restrict writeonly uniform image2D post_process_color;//output image

float read_depth_offset(vec2 center, ivec2 offset)
{
    float ndc_depth =  2.0*textureLod(depth_buffer,center + (vec2(offset)+0.5) / resolution.xy,0.0).x-1.0;
    return ndc_depth;
}

vec3 WorldPosFromCoord(vec2 n_coords){

	float depth = read_depth_offset(n_coords,ivec2(0,0));

	vec4 clipSpacePosition = vec4(n_coords * 2.0 - 1.0, depth, 1.0);
	return (inv_w_v_p * clipSpacePosition).xyz;
	}

void main(){

    vec4 white = vec4(1.0,1.0,1.0,1.0);
    vec4 black = vec4(0.0,0.0,0.0,1.0);

	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

	if (coord.x>=resolution.x || coord.y>=resolution.y)
        return; //Do not process out of screen

	vec2 n_coords = (vec2(coord) + 0.5) / resolution.xy;
        
        if(WorldPosFromCoord(n_coords).y < 0){
        // it is water, simulate
		imageStore(post_process_color,coord,white);
        }
        else  {
        // it is not water, do not simulate
        imageStore(post_process_color,coord,black);
        }

}
