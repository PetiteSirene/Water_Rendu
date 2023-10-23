#ifndef TEXTURE_HELPER
#define TEXTURE_HELPER

#include "helpers_common.hpp"
#include <stb_image.h>


class Texture2D {
public:
	Texture2D();
	~Texture2D();
	void set_format_params(const CpuGpuTransfertFormat format, const int levels_count = 0, const bool mipmap_auto = true);
	void set_filtering_params( const GLenum mag_filter = GL_LINEAR, GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR, const float max_anisotropy = 16.0f,const GLenum wrap_s = GL_REPEAT, const GLenum wrap_t = GL_REPEAT);
	void create_from_file(const std::string path_to_texture,const bool flip_y = true);
	void create_from_memory_base_level(const void* ptr, const glm::uvec2 size);
	void create_empty(const glm::uvec2 size);
	void re_create_empty(const glm::uvec2 size, const int levels_count = 1);
	void set_slot(const GLuint slot);
	void compute_mipmaps();
	void bind_to_image(GLenum access_flag = GL_READ_WRITE, int slot = -1);
	GLuint m_tex_id;
	glm::uvec2 m_size;
private:
	void get_levels_count(); // updates m_levels_count

	bool m_mipmap_auto;
	int m_levels_count;// if m_levels_count = 0 : compute level count based on size
	CpuGpuTransfertFormat m_format;
	GLenum m_mag_filter;
	GLenum m_min_filter;
	GLenum m_wrap_s;
	GLenum m_wrap_t;
	GLenum m_max_anisotropy;
	GLenum m_slot;
};

class Framebuffer {
public:
	Framebuffer();
	~Framebuffer();
	void create_framebuffer(const int color_attachment_count, const CpuGpuTransfertFormat format, const bool is_z_buffer = true);//Call once
	void bind_framebuffer();
	void update_size(const glm::uvec2 size);
	GLuint m_framebuffer_id;
	
private:
	CpuGpuTransfertFormat m_format;
	glm::uvec2 m_size;
	Texture2D* m_texture_channels;
	int m_color_attachment_count;
	bool m_is_z_buffer;
};

class Cubemap {
public:
	Cubemap();
	~Cubemap();
	void set_format_params(const CpuGpuTransfertFormat format, const int levels_count = 0, const bool mipmap_auto = true);
	void set_filtering_params(const GLenum mag_filter = GL_LINEAR, GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR, const float max_anisotropy = 32.0f);
	void create_from_file(const std::string path_to_file_prefix, const std::string extension, const bool flip_y = true,
		const std::string x_pos = "posx", const std::string x_neg = "negx",
		const std::string y_pos = "posy", const std::string y_neg = "negy",
		const std::string z_pos = "posz", const std::string z_neg = "negz");

	void create_from_memory_base_level(const void** ptr, const glm::uvec2 size);
	void create_empty(const glm ::uvec2 size);
	void set_slot(const GLuint slot);
	void compute_mipmaps();

private:
	void get_levels_count(); // updates m_levels_count
	GLuint m_tex_id;
	glm::uvec2 m_size;
	bool m_mipmap_auto;
	int m_levels_count;// if m_levels_count = 0 : compute level count based on size
	CpuGpuTransfertFormat m_format;
};





#endif