#ifndef HELPERS_COMMON
#define HELPERS_COMMON

#ifdef NDEBUG
#define OPENGL_RELEASE_CONTEXT 
#else
//debug context if not defined
#endif


#define GLM_FORCE_LEFT_HANDED // because of OpenGL NDC
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <glad/gl.h>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#define RESTART_PRIMITIVE_UINT 0xFFFFFFFF

struct CpuGpuTransfertFormat
{
	// formats lists at https://gist.github.com/Kos/4739337
	GLenum gpu_format;// internalformat (GL_RGBA8)
	GLenum cpu_channels;// format (GL_RGBA), do not forget _INTEGER for integer textures
	GLenum cpu_type;// type (GL_UNSIGNED_BYTE)
	int channel_count; // number of component (4)
};

#include "shader_helper.hpp"
#include "buffers_helper.hpp"
#include "debug_opengl.hpp"
#include "context_helper.hpp"
#include "camera_helper.hpp"
#include "texture_helper.hpp"
#endif