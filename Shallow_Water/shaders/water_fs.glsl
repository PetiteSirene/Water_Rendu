#version 460
#define UBO_APPLICATION_BINDING 0
layout (location = 0) out vec4 pixel_color;

//matches buffer_structures.hpp
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

in vec3 pos;


void main() 
{
    
    vec3 color = vec3(0.0 , 1.0 , 0.0);

    pixel_color = vec4(color,1.0);
}


vec3 get_sun_intensity()//To call for Phong shading
{
    return sun_light.w * mix(vec3(1.0,0.7,0.5), vec3(1.0,1.0,1.0), sun_light.y);
}
