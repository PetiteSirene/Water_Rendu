#version 460
#define UBO_APPLICATION_BINDING 0
layout (location = 0) out vec4 pixel_color;

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


in vec3 dir_ws;

vec3 environment_map(vec3 dir);

void main() 
{
    pixel_color = vec4(environment_map(normalize(dir_ws)),1.0);
    gl_FragDepth = 1.0f;//Force max depth
}

vec3 environment_map(vec3 dir)
{
    //Inspired from: https://www.shadertoy.com/view/MdtXD2
    dir.y = clamp(dir.y*0.85+0.15,0.0,1.0);
    float sun = max(1.0 - (1.0 + 10.0 * sun_light.y + 1.0 * dir.y) * length(sun_light.xyz - dir),0.0)
        + 0.3 * pow(1.0-dir.y,12.0) * (1.6-sun_light.y);
    return mix(vec3(0.3984,0.5117,0.7305), vec3(0.7031,0.4687,0.1055), sun)
              * ((0.5 + 1.0 * pow(sun_light.y,0.4)) * (1.5-dir.y) + pow(sun, 5.2)
              * sun_light.y * (5.0 + 15.0 * sun_light.y));

}