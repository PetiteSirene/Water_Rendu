#ifndef BUFFER_HELPER
#define BUFFER_HELPER

#include "helpers_common.hpp"

struct DrawElementsBaseVertexCommand {
	GLuint count;
	GLuint instance_count;
	GLuint first_index;
	GLuint base_vertex;
	GLuint base_instance;
};

class GPUBuffer {

public:
	GPUBuffer();
	~GPUBuffer();

	// Do not call this function for buffers feeding VAOs. In that case, bindings are done from the VAO side !
	//
	// targets: GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER
	// SLOT must match shaders binding: layout(binding = SLOT, ...
	void set_target_and_slot(GLenum target, GLuint slot);

	void allocate(GLsizeiptr size);
	void re_allocate(GLsizeiptr size);
	void write_to_gpu(const void* data, GLintptr offset = 0, GLsizeiptr size = 0);
	void* read_from_gpu(void* data = nullptr, GLintptr offset = 0, GLsizeiptr size = 0);
	void clear_to_value(CpuGpuTransfertFormat value_format, const void* clear_value = nullptr);

	GLuint m_buffer_id;
private:
	GLsizeiptr m_size;
	GLenum m_target;
	GLuint m_slot;
};

class VertexArrayObject {
public:
	VertexArrayObject();
	~VertexArrayObject();

	void use_vao();

	//component_count: float=1, glm::vec2=2, glm::vec3=3, glm::vec4=4
	//GLSL side: layout (location = shader_location_channel) in float/vecX ...
	void set_channel_float_type(GLuint shader_location_channel, GLuint vertex_buffer_id, GLsizei component_count, GLenum source_type = GL_FLOAT, GLboolean normalized = GL_FALSE);
	//for Draw*Indirect*
	void bind_indirect_command_buffer(GLuint command_buffer_id);
	//for Draw*Element*
	void bind_element_buffer(GLuint element_buffer_id);
private:
	GLuint m_element_buffer_id;
	GLuint m_command_buffer_id;
	GLuint m_vao_id;

};



#endif


