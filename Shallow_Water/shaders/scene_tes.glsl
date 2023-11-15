#version 460
#define UBO_APPLICATION_BINDING 0
layout(quads, equal_spacing, ccw) in;
in vec2 pos_2d[];

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

layout(binding = 8) uniform sampler2D tex_displacement;

out vec3 normal_ws;
out vec3 tangent_ws;
out vec3 bitangent_ws;
out vec3 pos_ws;
out vec2 tex_coords;


vec2 hash21(float p);
float fbm(vec2 uv,int octaves);
vec3 get_normal_ws_from_height(vec3 pos);
float tile_size = uintBitsToFloat(scene_params.z);
float offset_map = uintBitsToFloat(scene_params.w);



//Heightmap to generate 
float set_height(vec2 xz)//y axis 
{
    float factor = 3.0;
    float y = offset_map+fbm((xz+1000.0*hash21(scene_params.y))/factor,8)*factor*2.0;//random terrain
    y += 8.0*smoothstep(0.0,20.0,abs(xz.x - 5.0)-15.0);
    y += 24.0*smoothstep(0.0,30.0,abs(xz.y)-25.0);
    return y;
}

void main() 
{

    vec3 pos;
    pos.xz = tile_size*(pos_2d[0]+gl_TessCoord.xy);
    pos.y = set_height(pos.xz);

    tex_coords = gl_TessCoord.xy;
    normal_ws = get_normal_ws_from_height(pos);
    bitangent_ws = normalize(cross(normal_ws,vec3(1.0,0.0,0.0)));
    tangent_ws = cross(bitangent_ws,normal_ws);

    pos += vec3(0.0,0.3*texture(tex_displacement,gl_TessCoord.xy).x,0.0);//displacement mapping
    pos_ws = pos;
    gl_Position = w_v_p*vec4(pos,1.0);
}

///////////////////////
vec3 get_normal_ws_from_height(vec3 pos)
{
    vec2 dd = vec2(0.3,0.0);
    vec3 pdx;
    pdx.xz = pos.xz+dd.xy;
    pdx.y = set_height(pdx.xz);
    vec3 pdz;
    pdz.xz = pos.xz+dd.yx;
    pdz.y = set_height(pdz.xz);
    return normalize(cross(pdz-pos,pdx-pos));
}
vec2 hash21(float p)
{
    vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}
mat2 rotate2D(float r) {
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}
vec2 triwave(vec2 uv){
    return
        abs(fract(uv)-.5);
}
float fbm(vec2 uv,int octaves)
{
    //this function generates the terrain height
    float value = 0.,
    value1=value,
    amplitude = 1.5;
    uv /= 16.;
    vec2 t1 = vec2(0.);
    vec2 warp = vec2(0.);
    
    mat2 r = rotate2D(13.);
    vec2 uv1 = uv;
    for (int i = 0; i < octaves; i++)
    {
        t1 *= rotate2D(abs(t1.x+t1.y));
        t1= triwave(uv-triwave(uv1*r/2.15))-t1.yx;
        value1=sqrt(value1*value1+value*value+0.00001);
        value = (abs(t1.x-t1.y) * amplitude-value);
        amplitude /= 2.15;
        uv1 = uv;
        uv = (uv.yx*2.15 + t1)*r;
    }
    
    return value1-.9;
}