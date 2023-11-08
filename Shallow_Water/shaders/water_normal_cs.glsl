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
    vec4 water_params; //.x:resolution, .y:absorbance
};

layout(binding = 2,rgba32f) uniform image2D physicsTexture;

layout(binding = 3) writeonly uniform image2D normalsTexture;

void main(){
    float dist = 48.0f/ water_params.x; // a mettre dans l'ubo et sans le 48 hardcodé
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
  if (coord.x >= water_params.x || coord.y >= water_params.x)
        return; //Do not process out of screen

  //Copy texture XY to ZW
  vec2 data_src = imageLoad(physicsTexture,coord).xy;
  imageStore(physicsTexture,coord,data_src.xyxy);

    // tantpis pour les effets de bord
    float xdiff = imageLoad(physicsTexture,coord + ivec2(1,0)).x - imageLoad(physicsTexture,coord - ivec2(1,0)).x;

    float zdiff = imageLoad(physicsTexture,coord + ivec2(0,1)).x - imageLoad(physicsTexture,coord - ivec2(0,1)).x;

    vec3 xvec = vec3( 2 * dist, xdiff, 0.0); 
    vec3 zvec = vec3( 0.0, zdiff, 2 * dist);


    
    vec4 normal = vec4(normalize(cross(xvec,zvec)),0.0);
        
        
    imageStore(normalsTexture,coord,normal);
 

}