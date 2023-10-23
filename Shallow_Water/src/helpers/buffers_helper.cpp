#include "context_helper.hpp"

GPUBuffer::GPUBuffer()
{
	m_size = 0;
	m_buffer_id = 0;
	glCreateBuffers(1, &m_buffer_id);
}

GPUBuffer::~GPUBuffer()
{
	glDeleteBuffers(1, &m_buffer_id);
}

void GPUBuffer::set_target_and_slot(GLenum target, GLuint slot)
{
	m_target = target;
	m_slot = slot;
	glBindBufferBase(m_target, m_slot, m_buffer_id);
}

void GPUBuffer::allocate(GLsizeiptr size)
{
	// Will throw an error if called two times
	m_size = size;
	//Force buffer to he hosted on GPU
	glNamedBufferStorage(m_buffer_id, m_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

void GPUBuffer::re_allocate(GLsizeiptr size)
{
	m_size = size;
	glDeleteBuffers(1, &m_buffer_id);
	glCreateBuffers(1, &m_buffer_id);
	glNamedBufferStorage(m_buffer_id, m_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(m_target, m_slot, m_buffer_id);
}

void GPUBuffer::write_to_gpu(const void* data, GLintptr offset, GLsizeiptr size)
{
	if (size == 0)
		size = m_size;
	glNamedBufferSubData(m_buffer_id, offset, size, data);
}

void* GPUBuffer::read_from_gpu(void* data, GLintptr offset, GLsizeiptr size)
{
	if (size == 0)
		size = m_size;
	if (data == nullptr) // no cpu read bufffer given, creating one
		data = malloc(size); // need to be freed later by the program
	glGetNamedBufferSubData(m_buffer_id, offset, size, data);
	return data;
}

void GPUBuffer::clear_to_value(CpuGpuTransfertFormat value_format, const void* clear_value)
{
	//May trigger performance warning
	glClearNamedBufferData(m_buffer_id, value_format.gpu_format, value_format.cpu_channels, value_format.cpu_type, clear_value);
}
//VAO class
VertexArrayObject::VertexArrayObject()
{
	glCreateVertexArrays(1, &m_vao_id);
	m_command_buffer_id = 0;
	m_element_buffer_id = 0;
}

VertexArrayObject::~VertexArrayObject()
{
	//destructing a vao will not delete the buffers it uses
	glDeleteVertexArrays(1, &m_vao_id);

}

void VertexArrayObject::bind_element_buffer(GLuint element_buffer_id)
{
	m_element_buffer_id = element_buffer_id;
	glVertexArrayElementBuffer(m_vao_id, m_element_buffer_id);
}

void VertexArrayObject::bind_indirect_command_buffer(GLuint command_buffer_id)
{
	m_command_buffer_id = command_buffer_id;
}

void VertexArrayObject::use_vao()
{
	if (m_command_buffer_id > 0) // if there is a command buffer
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_command_buffer_id);
	}
	glBindVertexArray(m_vao_id);
}


void VertexArrayObject::set_channel_float_type(GLuint shader_location_channel, GLuint vertex_buffer_id, GLsizei component_count, GLenum source_type, GLboolean normalized)
{
	glEnableVertexArrayAttrib(m_vao_id, shader_location_channel);
	glVertexArrayAttribBinding(m_vao_id, shader_location_channel, shader_location_channel);
	glVertexArrayAttribFormat(m_vao_id, shader_location_channel, component_count, source_type, normalized, 0u);
	glVertexArrayVertexBuffer(m_vao_id, shader_location_channel, vertex_buffer_id, 0, component_count * sizeof(float));

}
