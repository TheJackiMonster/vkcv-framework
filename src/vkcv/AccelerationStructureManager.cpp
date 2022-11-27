
#include "AccelerationStructureManager.hpp"

#include "vkcv/Core.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {
	
	bool AccelerationStructureManager::init(Core &core, BufferManager &bufferManager) {
		if (!HandleManager<AccelerationStructureEntry, AccelerationStructureHandle>::init(core)) {
			return false;
		}
		
		m_bufferManager = &bufferManager;
		return true;
	}
	
	uint64_t AccelerationStructureManager::getIdFrom(const AccelerationStructureHandle &handle) const {
		return handle.getId();
	}
	
	AccelerationStructureHandle
	AccelerationStructureManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return AccelerationStructureHandle(id, destroy);
	}
	
	void AccelerationStructureManager::destroyById(uint64_t id) {
		auto &accelerationStructure = getById(id);
		
		if (accelerationStructure.m_accelerationStructure) {
			getCore().getContext().getDevice().destroyAccelerationStructureKHR(
					accelerationStructure.m_accelerationStructure,
					nullptr,
					getCore().getContext().getDispatchLoaderDynamic()
			);
			
			accelerationStructure.m_accelerationStructure = nullptr;
		}
		
		if (accelerationStructure.m_storageBuffer) {
			accelerationStructure.m_storageBuffer = BufferHandle();
		}
	}
	
	const BufferManager &AccelerationStructureManager::getBufferManager() const {
		return *m_bufferManager;
	}
	
	BufferManager &AccelerationStructureManager::getBufferManager() {
		return *m_bufferManager;
	}
	
	AccelerationStructureManager::AccelerationStructureManager() noexcept :
		HandleManager<AccelerationStructureEntry, AccelerationStructureHandle>(),
		m_bufferManager(nullptr) {}
	
	AccelerationStructureManager::~AccelerationStructureManager() noexcept {
		clear();
	}
	
	vk::AccelerationStructureKHR
	AccelerationStructureManager::getVulkanAccelerationStructure(
			const AccelerationStructureHandle &handle) const {
		auto &accelerationStructure = (*this) [handle];
		return accelerationStructure.m_accelerationStructure;
	}
	
	vk::Buffer AccelerationStructureManager::getVulkanBuffer(
			const AccelerationStructureHandle &handle) const {
		auto &accelerationStructure = (*this) [handle];
		return getBufferManager().getBuffer(accelerationStructure.m_storageBuffer);
	}
	
	static vk::Format getVertexFormat(GeometryVertexType vertexType) {
		switch (vertexType) {
			case GeometryVertexType::POSITION_FLOAT3:
				return vk::Format::eR32G32B32Sfloat;
			case GeometryVertexType::UNDEFINED:
				return vk::Format::eUndefined;
			default:
				vkcv_log(LogLevel::ERROR, "unknown Enum");
				return vk::Format::eUndefined;
		}
	}
	
	static vk::IndexType getIndexType(IndexBitCount indexByteCount) {
		switch (indexByteCount) {
			case IndexBitCount::Bit8:
				return vk::IndexType::eUint8EXT;
			case IndexBitCount::Bit16:
				return vk::IndexType::eUint16;
			case IndexBitCount::Bit32:
				return vk::IndexType::eUint32;
			default:
				vkcv_log(LogLevel::ERROR, "unknown Enum");
				return vk::IndexType::eNoneKHR;
		}
	}
	
	AccelerationStructureHandle AccelerationStructureManager::createAccelerationStructure(
			const std::vector<GeometryData> &geometryData) {
		std::vector<vk::AccelerationStructureGeometryKHR> geometries;
		std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> geometryInfos;
		std::vector<std::vector<vk::AccelerationStructureBuildRangeInfoKHR>> rangeInfos;
		
		auto& bufferManager = getBufferManager();
		
		vk::DeviceSize accelerationStructureSize = 0;
		vk::DeviceSize scratchBufferSize = 0;
		
		const auto &dynamicDispatch = getCore().getContext().getDispatchLoaderDynamic();
		
		geometries.reserve(geometryData.size());
		
		for (const GeometryData &data : geometryData) {
			const auto vertexBufferAddress = bufferManager.getBufferDeviceAddress(
					data.getVertexBufferBinding().buffer
			) + data.getVertexBufferBinding().offset;
			
			const auto indexBufferAddress = bufferManager.getBufferDeviceAddress(
					data.getIndexBuffer()
			);
			
			const auto vertexStride = data.getVertexStride();
			const auto vertexBufferSize = bufferManager.getBufferSize(
					data.getVertexBufferBinding().buffer
			);
			
			const auto vertexCount = (vertexBufferSize / vertexStride);
			
			const vk::Format vertexFormat = getVertexFormat(data.getGeometryVertexType());
			
			const vk::IndexType indexType = getIndexType(data.getIndexBitCount());
			
			const vk::AccelerationStructureGeometryTrianglesDataKHR asTrianglesData (
					vertexFormat,
					vertexBufferAddress,
					vertexStride,
					static_cast<uint32_t>(vertexCount - 1),
					indexType,
					indexBufferAddress,
					{}
			);
			
			const vk::AccelerationStructureGeometryKHR asGeometry (
					vk::GeometryTypeKHR::eTriangles,
					asTrianglesData,
					vk::GeometryFlagBitsKHR::eOpaque
			);
			
			geometries.push_back(asGeometry);
		}
		
		geometryInfos.reserve(geometries.size());
		rangeInfos.reserve(geometries.size());
		
		for (size_t i = 0; i < geometries.size(); i++) {
			const vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo(
					vk::AccelerationStructureTypeKHR::eBottomLevel,
					vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
					vk::BuildAccelerationStructureModeKHR::eBuild,
					{},
					{},
					1,
					&(geometries[i])
			);
			
			geometryInfos.push_back(asBuildGeometryInfo);
			
			const vk::AccelerationStructureBuildRangeInfoKHR asBuildRangeInfo (
					static_cast<uint32_t>(geometryData[i].getCount() / 3),
					0,
					0,
					0
			);
			
			rangeInfos.push_back({ asBuildRangeInfo });
			
			vk::AccelerationStructureBuildSizesInfoKHR asBuildSizesInfo;
			getCore().getContext().getDevice().getAccelerationStructureBuildSizesKHR(
					vk::AccelerationStructureBuildTypeKHR::eDevice,
					&(asBuildGeometryInfo),
					&(asBuildRangeInfo.primitiveCount),
					&(asBuildSizesInfo),
					dynamicDispatch
			);
			
			accelerationStructureSize += asBuildSizesInfo.accelerationStructureSize;
			scratchBufferSize = std::max(scratchBufferSize, asBuildSizesInfo.buildScratchSize);
		}
		
		const BufferHandle &asStorageBuffer = bufferManager.createBuffer(
				typeGuard<uint8_t>(),
				BufferType::ACCELERATION_STRUCTURE_STORAGE,
				BufferMemoryType::DEVICE_LOCAL,
				accelerationStructureSize,
				false
		);
		
		vk::PhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties;
		
		vk::PhysicalDeviceProperties2 physicalProperties2;
		physicalProperties2.pNext = &accelerationStructureProperties;
		
		getCore().getContext().getPhysicalDevice().getProperties2(&physicalProperties2);
		
		const auto minScratchAlignment = (
				accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment
		);
		
		const BufferHandle &asScratchBuffer = bufferManager.createBuffer(
				vkcv::TypeGuard(minScratchAlignment),
				BufferType::STORAGE,
				BufferMemoryType::DEVICE_LOCAL,
				(scratchBufferSize + minScratchAlignment - 1) / minScratchAlignment,
				false
		);
		
		if ((!asStorageBuffer) || (!asScratchBuffer)) {
			return {};
		}
		
		const vk::AccelerationStructureCreateInfoKHR asCreateInfo (
				vk::AccelerationStructureCreateFlagsKHR(),
				bufferManager.getBuffer(asStorageBuffer),
				0,
				accelerationStructureSize,
				vk::AccelerationStructureTypeKHR::eBottomLevel
		);
		
		vk::AccelerationStructureKHR accelerationStructure;
		const vk::Result result = getCore().getContext().getDevice().createAccelerationStructureKHR(
				&asCreateInfo,
				nullptr,
				&accelerationStructure,
				dynamicDispatch
		);
		
		if (result != vk::Result::eSuccess) {
			return {};
		}
		
		vk::DeviceAddress scratchBufferAddress = bufferManager.getBufferDeviceAddress(
				asScratchBuffer
		);
		
		if (scratchBufferAddress % minScratchAlignment != 0) {
			scratchBufferAddress += (
					minScratchAlignment - (scratchBufferAddress % minScratchAlignment)
			);
		}
		
		for (auto& geometryInfo : geometryInfos) {
			geometryInfo.setDstAccelerationStructure(accelerationStructure);
			geometryInfo.setScratchData(scratchBufferAddress);
		}
		
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> pRangeInfos;
		pRangeInfos.resize(rangeInfos.size());
		
		for (size_t i = 0; i < rangeInfos.size(); i++) {
			pRangeInfos[i] = rangeInfos[i].data();
		}
		
		auto cmdStream = getCore().createCommandStream(vkcv::QueueType::Compute);
		
		getCore().recordCommandsToStream(
				cmdStream,
				[&geometryInfos, &pRangeInfos, &dynamicDispatch](
						const vk::CommandBuffer &cmdBuffer) {
			cmdBuffer.buildAccelerationStructuresKHR(
					static_cast<uint32_t>(geometryInfos.size()),
					geometryInfos.data(),
					pRangeInfos.data(),
					dynamicDispatch
			);
		}, nullptr);
		
		getCore().submitCommandStream(cmdStream, false);
		
		return add({ accelerationStructure, asStorageBuffer });
	}
	
}
