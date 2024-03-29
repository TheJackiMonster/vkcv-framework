
#include "vkcv/VertexData.hpp"

namespace vkcv {

	VertexBufferBinding vertexBufferBinding(const BufferHandle &buffer,
											size_t stride,
											size_t offset) {
		VertexBufferBinding binding;
		binding.m_buffer = buffer;
		binding.m_stride = stride;
		binding.m_offset = offset;
		return binding;
	}

	VertexData::VertexData(const Vector<VertexBufferBinding> &bindings) :
		m_bindings(bindings), m_indices(), m_indexBitCount(IndexBitCount::Bit16), m_count(0) {}

	const Vector<VertexBufferBinding> &VertexData::getVertexBufferBindings() const {
		return m_bindings;
	}

	void VertexData::setIndexBuffer(const BufferHandle &indices, IndexBitCount indexBitCount) {
		m_indices = indices;
		m_indexBitCount = indexBitCount;
	}

	const BufferHandle &VertexData::getIndexBuffer() const {
		return m_indices;
	}

	IndexBitCount VertexData::getIndexBitCount() const {
		return m_indexBitCount;
	}

	void VertexData::setCount(size_t count) {
		m_count = count;
	}

	size_t VertexData::getCount() const {
		return m_count;
	}

} // namespace vkcv
