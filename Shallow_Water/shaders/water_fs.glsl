#version 460
#define UBO_APPLICATION_BINDING 0
layout (location = 0) out vec4 pixel_color;

//matches buffer_structures.hpp
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

in vec3 pos;
in vec2 tex_coords;//in [0.0,1.0]
layout(binding = 0) uniform sampler2D tex_altitude;
layout(binding = 3) uniform sampler2D normalTexture;

vec3 absorption_subs = vec3(1.0,1.0,1.0) - water_absorption_color.xyz;
float absorbance = water_rendering_params.y;
float refractive_ratio = 1/water_rendering_params.x;

float marching_step = water_raymarching_params.x;
float d_min = water_raymarching_params.y;
float d_max = water_raymarching_params.z;
int max_step = floatBitsToInt(water_raymarching_params.w);

float raymarch_distance(vec3 p)
{
    vec2 tex_coords = (pos.xz + scene_params.x * uintBitsToFloat(scene_params.z) * 0.5) / (water_sim_params.w * float(resolution.w));
    return p.y - textureLod(tex_altitude, tex_coords,0.0).x;
}
float raymarch(vec3 origin, vec3 dir)
{    
    float t = 0.0;
    for (int i=0; i<max_step; i++)
    {
        vec3 p = origin + t*dir;
        float d = raymarch_distance(p);
        if( d < d_min || t > d_max) break;
        t += marching_step;
    }
    return t;
}

void main() 
{    
    // computing refracted ray
    vec3 N = textureLod(normalTexture,tex_coords,0.0).xyz;
    vec3 I = normalize(pos - cam_pos.xyz);
    vec3 R = refract(I,N,refractive_ratio);
    float t = 0.0;
    // raymarching the refracted ray
    t = raymarch(pos,R);
    vec3 color = 0.5*R+0.5;
    color = vec3(1.0,1.0,1.0);
    color -= absorption_subs * absorbance * raymarch(pos,R);

    pixel_color = vec4(color,1.0);
}




vec3 get_sun_intensity()//To call for Phong shading
{
    return sun_light.w * mix(vec3(1.0,0.7,0.5), vec3(1.0,1.0,1.0), sun_light.y);
}
