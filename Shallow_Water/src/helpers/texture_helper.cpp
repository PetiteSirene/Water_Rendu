#include "texture_helper.hpp"
Texture2D::Texture2D()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &m_tex_id);
	m_size = glm::uvec2(0);
	m_mipmap_auto = true;
	m_levels_count = 0;
	m_format = CpuGpuTransfertFormat({ 0 });
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_tex_id);
}

void Texture2D::set_format_params(const CpuGpuTransfertFormat format, const int levels_count, const bool mipmap_auto)
{
	m_levels_count = levels_count;
	m_mipmap_auto = mipmap_auto;
	m_format = format;
}

void Texture2D::set_filtering_params(const GLenum mag_filter, GLenum min_filter, const float max_anisotropy, const GLenum wrap_s, const GLenum wrap_t)
{
	m_mag_filter = mag_filter;
	m_min_filter = min_filter;
	m_max_anisotropy = max_anisotropy;
	m_wrap_s = wrap_s;
	m_wrap_t = wrap_t;
	glTextureParameteri(m_tex_id, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTextureParameteri(m_tex_id, GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(m_tex_id, GL_TEXTURE_WRAP_S, wrap_s);
	glTextureParameteri(m_tex_id, GL_TEXTURE_WRAP_T, wrap_t);

	if (max_anisotropy > 1.0)
	{
		float device_max_anisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &device_max_anisotropy);
		glTextureParameteri(m_tex_id, GL_TEXTURE_MAX_ANISOTROPY, glm::min(device_max_anisotropy, max_anisotropy));
	}
}

void Texture2D::create_from_file(const std::string path_to_texture,const bool flip_y)
{
	stbi_set_flip_vertically_on_load(flip_y);
	int x, y, channels;
	stbi_uc* tex_memory = stbi_load(path_to_texture.c_str(), &x, &y, &channels, m_format.channel_count);
	if (tex_memory == nullptr)
	{
		std::cout << "ERROR: Could not read " << path_to_texture << std::endl;
	}
	m_size.x = x;
	m_size.y = y;

	create_from_memory_base_level(tex_memory, m_size);
	delete tex_memory;

}

void Texture2D::create_from_memory_base_level(const void* ptr, const  glm::uvec2 size)
{
	m_size = size;
	create_empty(m_size);

	glTextureSubImage2D(m_tex_id, 0, 0, 0, m_size.x, m_size.y, m_format.cpu_channels, m_format.cpu_type, ptr);
	if (m_mipmap_auto && m_levels_count > 1) // compute mipmaps
	{
		compute_mipmaps();
	}

}

void Texture2D::create_empty(const glm::uvec2 size)
{
	m_size = size;
	get_levels_count(); // performs mipmap levels computation if necessary
	glTextureStorage2D(m_tex_id, m_levels_count, m_format.gpu_format, m_size.x, m_size.y);

}

void Texture2D::re_create_empty(const glm::uvec2 size, const int levels_count)
{
	m_levels_count = levels_count;
	glDeleteTextures(1, &m_tex_id);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_tex_id);
	m_size = size;
	get_levels_count(); // performs mipmap levels computation if necessary
	glTextureStorage2D(m_tex_id, m_levels_count, m_format.gpu_format, m_size.x, m_size.y);
	set_slot(m_slot);
	set_format_params(m_format, m_levels_count, m_mipmap_auto);
	set_filtering_params(m_mag_filter, m_min_filter, m_max_anisotropy, m_wrap_s, m_wrap_t);


}


void Texture2D::set_slot(const GLuint slot)
{
	m_slot = slot;
	glBindTextureUnit(slot, m_tex_id);
}

void Texture2D::compute_mipmaps()
{
	glGenerateTextureMipmap(m_tex_id);
}

void Texture2D::bind_to_image(GLenum access_flag, int slot)
{

	glBindImageTexture(slot==-1?m_slot: slot,m_tex_id,0, GL_TRUE,0,access_flag,m_format.gpu_format);
}

void Texture2D::get_levels_count()
{
	if (m_levels_count != 0)
		return;

	m_levels_count = 1 + (int)glm::log2((float)std::min(m_size.x, m_size.y));
}


Framebuffer::Framebuffer()
{
	m_size = glm::uvec2(0xFFFFFFFF, 0xFFFFFFFF);
}
Framebuffer::~Framebuffer()
{
	delete[] m_texture_channels;
	glDeleteFramebuffers(1, &m_framebuffer_id);
}

void Framebuffer::create_framebuffer(const int color_attachment_count, const CpuGpuTransfertFormat format, const bool is_z_buffer)
{
	m_format = format;
	m_color_attachment_count = color_attachment_count;
	m_is_z_buffer = is_z_buffer;
	glCreateFramebuffers(1, &m_framebuffer_id);
	m_texture_channels = new Texture2D[color_attachment_count + int(is_z_buffer)];

	GLenum *draw_buffers = new GLenum[color_attachment_count];
	if (color_attachment_count > 0)
	{
		for (int i = 0; i < color_attachment_count; i++)
			draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
		glNamedFramebufferDrawBuffers(m_framebuffer_id, color_attachment_count, draw_buffers);
	}
}

void Framebuffer::bind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
}

void Framebuffer::update_size(const glm::uvec2 size)
{
	if (size.x == 0 && size.y == 0)//Do nothing if windows is minimized
		return;

	if (m_size.x == 0xFFFFFFFF && m_size.y == 0xFFFFFFFF)//Creation of textures
	{
		for (int i = 0; i < m_color_attachment_count; i++)
		{
			m_texture_channels[i].set_format_params(m_format, 1);//requires 4 channels to bind to image (for Load/Store)
			m_texture_channels[i].create_empty(glm::uvec2(16, 16));
			m_texture_channels[i].set_filtering_params(GL_LINEAR, GL_LINEAR, 0.0f, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			m_texture_channels[i].set_slot(i);
		}
		if (m_is_z_buffer)
		{
			m_texture_channels[m_color_attachment_count].set_format_params({ GL_DEPTH_COMPONENT32,GL_DEPTH_COMPONENT ,GL_UNSIGNED_INT ,1 }, 1);//requires 4 channels to bind to image (for Load/Store)
			m_texture_channels[m_color_attachment_count].create_empty(glm::uvec2(16, 16));
			m_texture_channels[m_color_attachment_count].set_filtering_params(GL_LINEAR, GL_LINEAR, 0.0f, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			m_texture_channels[m_color_attachment_count].set_slot(m_color_attachment_count);
		}
	}

	for (int i = 0; i < m_color_attachment_count; i++)
	{
		m_texture_channels[i].re_create_empty(size, 1);
		glNamedFramebufferTexture(m_framebuffer_id, GL_COLOR_ATTACHMENT0 + i, m_texture_channels[i].m_tex_id, 0);
	}
	if (m_is_z_buffer)
	{
	m_texture_channels[m_color_attachment_count].re_create_empty(size, 1);
	glNamedFramebufferTexture(m_framebuffer_id, GL_DEPTH_ATTACHMENT, m_texture_channels[m_color_attachment_count].m_tex_id, 0);
	}
	

	if (glCheckNamedFramebufferStatus(m_framebuffer_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Incomplet Framebuffer" << std::endl;
	}


	m_size = size;
}

Cubemap::Cubemap()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_tex_id);
	m_size = glm::uvec2(0);
	m_mipmap_auto = true;
	m_levels_count = 0;
	m_format = CpuGpuTransfertFormat({ 0 });
}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &m_tex_id);
}

void Cubemap::set_format_params(const CpuGpuTransfertFormat format, const int levels_count, const bool mipmap_auto)
{
	m_levels_count = levels_count;
	m_mipmap_auto = mipmap_auto;
	m_format = format;
}

void Cubemap::set_filtering_params(const GLenum mag_filter, GLenum min_filter, const float max_anisotropy)
{
	glTextureParameteri(m_tex_id, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTextureParameteri(m_tex_id, GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(m_tex_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_tex_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_tex_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	float device_max_anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &device_max_anisotropy);

	glTextureParameteri(m_tex_id, GL_TEXTURE_MAX_ANISOTROPY, glm::min(device_max_anisotropy, max_anisotropy));
}
void Cubemap::create_from_file(const std::string path_to_file_prefix, const std::string extension, const bool flip_y,
	const std::string x_pos, const std::string x_neg,
	const std::string y_pos, const std::string y_neg,
	const std::string z_pos, const std::string z_neg)
{
	stbi_set_flip_vertically_on_load(flip_y);

	std::string faces_filename[6];
	faces_filename[0] = path_to_file_prefix + x_pos + extension;
	faces_filename[1] = path_to_file_prefix + x_neg + extension;
	faces_filename[2] = path_to_file_prefix + y_pos + extension;
	faces_filename[3] = path_to_file_prefix + y_neg + extension;
	faces_filename[4] = path_to_file_prefix + z_pos + extension;
	faces_filename[5] = path_to_file_prefix + z_neg + extension;


	int x, y, channels;
	stbi_uc* tex_memory[6];
	for (int face = 0; face < 6; face++)
	{
		tex_memory[face] = stbi_load(faces_filename[face].c_str(), &x, &y, &channels, m_format.channel_count);
		if (tex_memory[face] == nullptr)
		{
			std::cout << "ERROR: Could not read " << faces_filename[face] << std::endl;
		}
	}
	m_size.x = x;
	m_size.y = y;

	create_from_memory_base_level((const void**)(tex_memory), m_size);
	for (int face = 0; face < 6; face++)
	{
		delete tex_memory[face];
	}

}

void Cubemap::create_from_memory_base_level(const void** ptr, const  glm::uvec2 size)
{
	m_size = size;
	create_empty(m_size);
	for (int face = 0; face < 6; face++)
		glTextureSubImage3D(m_tex_id, 0, 0, 0, face, m_size.x, m_size.y, 1, m_format.cpu_channels, m_format.cpu_type, ptr[face]);

	if (m_mipmap_auto && m_levels_count > 1) // compute mipmaps
	{
		compute_mipmaps();
	}

}

void Cubemap::create_empty(const glm::uvec2 size)
{
	m_size = size;
	get_levels_count(); // performs mipmap levels computation if necessary
	glTextureStorage2D(m_tex_id, m_levels_count, m_format.gpu_format, m_size.x, m_size.y);

}


void Cubemap::set_slot(const GLuint slot)
{
	glBindTextureUnit(slot, m_tex_id);
}

void Cubemap::compute_mipmaps()
{
	glGenerateTextureMipmap(m_tex_id);
}

void Cubemap::get_levels_count()
{
	if (m_levels_count != 0)
		return;

	m_levels_count = 1 + (int)glm::log2((float)std::min(m_size.x, m_size.y));
}



