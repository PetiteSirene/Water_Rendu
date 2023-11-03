#version 460
#define UBO_APPLICATION_BINDING 0
layout(quads, equal_spacing, ccw) in;
in vec2 ijCoords[];

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
    vec4 water_params; //.x:resolution, .y:absorbance
};

layout(binding = 2, rg8) restrict readonly uniform image2D physicsImage;

out vec3 pos;

float tile_size = uintBitsToFloat(scene_params.z);

void main() 
{

    vec3 pos;
    pos.xz = tile_size*(ijCoords[0] - scene_params.x * 0.5 + gl_TessCoord.xy);
    pos.y = 0.0f;//imageLoad(physicsImage,ijCoords);
    gl_Position = w_v_p * vec4(pos,1.0);
}