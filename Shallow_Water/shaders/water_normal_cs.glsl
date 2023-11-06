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

layout(binding = 2,rg32f) uniform image2D physicsTexture;

layout(binding = 3) writeonly uniform image2D normalsTexture;

vec4 crossProduct(vec4 v1, vec4 v2) {
    double resultX = v1.y * v2.z - v1.z * v2.y;
    double resultY = v1.z * v2.x - v1.x * v2.z;
    double resultZ = v1.x * v2.y - v1.y * v2.x;

    return vec4(resultX, resultY, resultZ, 0.0);
}


void main(){
    float dist = 48.0f/ water_params.x; // a mettre dans l'ubo et sans le 48 hardcodé
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

    // tantpis pour les effets de bord
    float xdiff = imageLoad(physicsTexture,coord + ivec2(1,0)).x - imageLoad(physicsTexture,coord - ivec2(1,0)).x;

    float zdiff = imageLoad(physicsTexture,coord + ivec2(0,1)).x - imageLoad(physicsTexture,coord - ivec2(0,1)).x;

    vec4 xvec = vec4( 2 * dist, xdiff, 0.0f, 0.0f); 
    vec4 zvec = vec4( 0.0f, zdiff, 2 * dist, 0.0f);

	if (coord.x >= water_params.x || coord.y >= water_params.x)
        return; //Do not process out of screen
    
    vec4 normal = vec4(normalize(cross(xvec.xyz,zvec.xyz)),0.0); // inverser les vec si ca marche pas
        
        
    imageStore(normalsTexture,coord,normal);
 

}