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


in vec3 normal_ws;
in vec3 tangent_ws;
in vec3 bitangent_ws;
in vec3 pos_ws;
in vec2 tex_coords;

layout(binding = 0) uniform sampler2D tex_altitude;
layout(binding = 4) uniform sampler2D tex_albedo;
layout(binding = 5) uniform sampler2D tex_normal;
layout(binding = 6) uniform sampler2D tex_roughness;
layout(binding = 7) uniform sampler2D tex_ao;

vec3 get_normal();//bump mapping
vec3 get_sun_intensity();//To call for Phong shading

float scene_size_xz = uintBitsToFloat(scene_params.z)*scene_params.x;
void main() 
{
    float high_slope = clamp(6.0*(normal_ws.y-0.4),0.0,1.0);
    high_slope = mix(high_slope,1.0,clamp(1.0-pos_ws.y,0.0,1.0));
    float low_slope = clamp((1.1-normal_ws.y*1.2)*5.0,0.0,1.0);
    low_slope = mix(low_slope,1.0,clamp(1.0-pos_ws.y,0.0,1.0));
    float alt_slope = min(low_slope,high_slope);
    vec3 k = mix(0.2*vec3(1.0,0.5,1.0),texture(tex_albedo,tex_coords).rgb,high_slope);
    k = mix(0.9*vec3(1.0,1.0,1.0),k,low_slope);

    vec3 n = mix(normal_ws,get_normal(),alt_slope);
    float roughness = 1.0-mix(0.5,texture(tex_roughness,tex_coords).x,alt_slope);
    float ao = mix(1.0,texture(tex_ao,tex_coords).x,alt_slope);


    vec3 color = vec3(0.0);
    vec3 I = get_sun_intensity();
    vec3 v = normalize(cam_pos.xyz - pos_ws);
    vec3 l = normalize(sun_light.xyz);//already point outward
    vec3 r = normalize(-reflect(l,n));
    float d = dot(n,l);
    color += I * k * (0.3+0.7*max(0.0,d));//Ambient + diffuse
    color += I * 0.3*roughness* pow(clamp(dot(v,r)*step(0.0,d),0.0,1.0),1.5);
    color = clamp(color,0.0,1.0);
    color *= ao;//pre-baked ambient occlusion
    
    //color = mix(color,vec3(0.1,0.5,0.8),0.5*step(pos_ws.y,0.0));//blue if < 0.0

    //color = vec3(texture(tex_altitude,pos_ws.xz/scene_size_xz+0.5).x);
    pixel_color = vec4(color,1.0);//ID of ground is 1
}

////
vec3 get_normal()
{
    vec3 n = normalize(normal_ws);
    vec3 t = normalize(tangent_ws);
    vec3 bt = normalize(bitangent_ws);
    vec3 bump = 2.0*texture(tex_normal,tex_coords).xyz-1.0;
    return normalize(t*bump.x+bt*bump.y+n*bump.z);
}

vec3 get_sun_intensity()//To call for Phong shading
{
    return sun_light.w * mix(vec3(1.0,0.7,0.5), vec3(1.0,1.0,1.0), sun_light.y);
}
