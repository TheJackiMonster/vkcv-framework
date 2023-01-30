/**
 * @authors Tobias Frisch
 * @file src/vkcv/Drawcall.cpp
 */

#include "vkcv/Drawcall.hpp"

namespace vkcv {

	const std::vector<DescriptorSetUsage> &Drawcall::getDescriptorSetUsages() const {
		return m_usages;
	}

	void Drawcall::useDescriptorSet(uint32_t location, const DescriptorSetHandle &descriptorSet,
									const std::vector<uint32_t> &dynamicOffsets) {
		DescriptorSetUsage usage;
		usage.location = location;
		usage.descriptorSet = descriptorSet;
		usage.dynamicOffsets = dynamicOffsets;
		m_usages.push_back(usage);
	}

	InstanceDrawcall::InstanceDrawcall(const VertexData &vertexData,
									   uint32_t instanceCount) :
		Drawcall(),
		m_vertexData(vertexData),
		m_instanceCount(instanceCount) {}

	const VertexData &InstanceDrawcall::getVertexData() const {
		return m_vertexData;
	}

	uint32_t InstanceDrawcall::getInstanceCount() const {
		return m_instanceCount;
	}

	IndirectDrawcall::IndirectDrawcall(const BufferHandle &indirectDrawBuffer,
									   const VertexData &vertexData,
									   uint32_t drawCount) :
		Drawcall(),
		m_indirectDrawBuffer(indirectDrawBuffer),
		m_vertexData(vertexData),
		m_drawCount(drawCount) {
	}

	BufferHandle IndirectDrawcall::getIndirectDrawBuffer() const {
		return m_indirectDrawBuffer;
	}

	const VertexData &IndirectDrawcall::getVertexData() const {
		return m_vertexData;
	}

	uint32_t IndirectDrawcall::getDrawCount() const {
		return m_drawCount;
	}

	TaskDrawcall::TaskDrawcall(const DispatchSize& taskSize) : Drawcall(), m_taskSize(taskSize) {}
	
	DispatchSize TaskDrawcall::getTaskSize() const {
		return m_taskSize;
	}

} // namespace vkcv
