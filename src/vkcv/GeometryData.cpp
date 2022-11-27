
#include "vkcv/GeometryData.hpp"

namespace vkcv {
	
	GeometryData::GeometryData() :
		m_vertexBinding({}),
		m_vertexStride(0),
		m_vertexType(GeometryVertexType::UNDEFINED),
		m_indices(),
		m_indexBitCount(IndexBitCount::Bit16),
		m_count(0) {}
	
	GeometryData::GeometryData(const VertexBufferBinding &binding,
							   uint32_t stride,
							   GeometryVertexType geometryVertexType) :
		m_vertexBinding(binding),
		m_vertexStride(stride),
		m_vertexType(geometryVertexType),
		m_indices(),
		m_indexBitCount(IndexBitCount::Bit16),
		m_count(0) {}
	
	bool GeometryData::isValid() const {
		return m_vertexType == GeometryVertexType::UNDEFINED;
	}
	
	const VertexBufferBinding &GeometryData::getVertexBufferBinding() const {
		return m_vertexBinding;
	}
	
	uint32_t GeometryData::getVertexStride() const {
		return m_vertexStride;
	}
	
	GeometryVertexType GeometryData::getGeometryVertexType() const {
		return m_vertexType;
	}
	
	void GeometryData::setIndexBuffer(const BufferHandle &indices, IndexBitCount indexBitCount) {
		m_indices = indices;
		m_indexBitCount = indexBitCount;
	}
	
	const BufferHandle &GeometryData::getIndexBuffer() const {
		return m_indices;
	}
	
	IndexBitCount GeometryData::getIndexBitCount() const {
		return m_indexBitCount;
	}
	
	void GeometryData::setCount(size_t count) {
		m_count = count;
	}
	
	size_t GeometryData::getCount() const {
		return m_count;
	}
	
}
