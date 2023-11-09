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
  ivec4 resolution; //.x:resolution.x, .y:resolution.y, .z: debug_mode, .w: sim_resolution
  vec4 sun_light;//.xyz: direction, .w:intensity
  uvec4 scene_params; //.x:tile_count, .y:seed, .z:tile_size[FLOAT], .w:map_offset[FLOAT]
  vec4 water_sim_params; //.x:celerity, .y: damping, .z:delta_time, .w: sim_tile_length
  vec4 water_rendering_params;
  vec4 water_aborption_color;
};

layout(binding = 2,rgba32f) uniform restrict image2D physicsTexture;

void main(){
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
  if (coord.x >= resolution.w || coord.y >= resolution.w)
    return; //Do not process out of simulation texture

  //Copy texture XY to ZW
  vec2 data_src = imageLoad(physicsTexture,coord).xy;
  imageStore(physicsTexture,coord,data_src.xyxy);
}