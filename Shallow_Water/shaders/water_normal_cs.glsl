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

layout(r32f,binding = 0) uniform readonly restrict image2D tex_altitude_ws;
layout(binding = 2,rgba32f) uniform image2D physicsTexture;

layout(binding = 3) writeonly uniform image2D normalsTexture;

void main(){

  ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
  if (coord.x >= resolution.w || coord.y >= resolution.w)
    return;

  //write water perturbation
  if (water_rendering_params.w==1.0)
  {
    ivec2 coord_water = ivec2(resolution.w * unpackUnorm2x16(uint(resolution.z)));
    if (imageLoad(tex_altitude_ws, coord_water).x<0.0)
    {
      vec2 disturb = vec2(water_aborption_color.w);
      imageStore(physicsTexture,coord_water,disturb.xyxy);
    }
  }

  float dist = water_sim_params.w;

  // tantpis pour les effets de bord
  float xdiff = imageLoad(physicsTexture,coord + ivec2(1,0)).x - imageLoad(physicsTexture,coord - ivec2(1,0)).x;

  float zdiff = imageLoad(physicsTexture,coord + ivec2(0,1)).x - imageLoad(physicsTexture,coord - ivec2(0,1)).x;

  vec3 xvec = vec3( 2 * dist, xdiff, 0.0); 
  vec3 zvec = vec3( 0.0, zdiff, 2 * dist);

  vec4 normal = vec4(normalize(cross(zvec,xvec)),0.0);

  imageStore(normalsTexture,coord,normal);
}