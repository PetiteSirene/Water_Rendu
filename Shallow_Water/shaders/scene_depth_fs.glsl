#version 460
#define UBO_APPLICATION_BINDING 0
layout (location = 0) out float altitude;

in vec3 normal_ws;
in vec3 tangent_ws;
in vec3 bitangent_ws;
in vec3 pos_ws;
in vec2 tex_coords;


void main() 
{
    altitude = pos_ws.y;
}

