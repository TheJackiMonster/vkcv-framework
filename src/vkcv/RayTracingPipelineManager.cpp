
#include "RayTracingPipelineManager.hpp"

#include "vkcv/Core.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {
	
	uint64_t RayTracingPipelineManager::getIdFrom(const RayTracingPipelineHandle &handle) const {
		return handle.getId();
	}
	
	RayTracingPipelineHandle RayTracingPipelineManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return RayTracingPipelineHandle(id, destroy);
	}
	
	void RayTracingPipelineManager::destroyById(uint64_t id) {
		auto &pipeline = getById(id);
		
		if (pipeline.m_shaderBindingTable) {
			pipeline.m_shaderBindingTable = BufferHandle();
		}
		
		if (pipeline.m_handle) {
			getCore().getContext().getDevice().destroy(pipeline.m_handle);
			pipeline.m_handle = nullptr;
		}
		
		if (pipeline.m_layout) {
			getCore().getContext().getDevice().destroy(pipeline.m_layout);
			pipeline.m_layout = nullptr;
		}
	}
	
	RayTracingPipelineManager::RayTracingPipelineManager() noexcept :
		HandleManager<RayTracingPipelineEntry, RayTracingPipelineHandle>() {}
	
	RayTracingPipelineManager::~RayTracingPipelineManager() noexcept {
		clear();
	}
	
	static vk::ShaderStageFlagBits shaderStageToVkShaderStage(ShaderStage stage) {
		switch (stage) {
			case ShaderStage::RAY_GEN:
				return vk::ShaderStageFlagBits::eRaygenKHR;
			case ShaderStage::RAY_ANY_HIT:
				return vk::ShaderStageFlagBits::eAnyHitKHR;
			case ShaderStage::RAY_CLOSEST_HIT:
				return vk::ShaderStageFlagBits::eClosestHitKHR;
			case ShaderStage::RAY_MISS:
				return vk::ShaderStageFlagBits::eMissKHR;
			case ShaderStage::RAY_INTERSECTION:
				return vk::ShaderStageFlagBits::eIntersectionKHR;
			case ShaderStage::RAY_CALLABLE:
				return vk::ShaderStageFlagBits::eCallableKHR;
			default:
				vkcv_log(LogLevel::ERROR, "Unknown shader stage");
				return vk::ShaderStageFlagBits::eAll;
		}
	}
	
	static bool createPipelineShaderStageCreateInfo(
			const ShaderProgram &shaderProgram,
			ShaderStage stage,
			vk::Device device,
			vk::PipelineShaderStageCreateInfo* outCreateInfo) {
		
		assert(outCreateInfo);
		std::vector<uint32_t> code = shaderProgram.getShaderBinary(stage);
		vk::ShaderModuleCreateInfo vertexModuleInfo(
				{},
				code.size() * sizeof(uint32_t),
				code.data()
		);
		
		vk::ShaderModule shaderModule;
		if (device.createShaderModule(&vertexModuleInfo, nullptr, &shaderModule)
			!= vk::Result::eSuccess)
			return false;
		
		const static auto entryName = "main";
		
		*outCreateInfo = vk::PipelineShaderStageCreateInfo(
				{},
				shaderStageToVkShaderStage(stage),
				shaderModule,
				entryName,
				nullptr
		);
		
		return true;
	}
	
	/**
	 * Creates a Pipeline Layout Create Info Struct.
	 * @param config sets Push Constant Size and Descriptor Layouts.
	 * @return Pipeline Layout Create Info Struct
	 */
	static vk::PipelineLayoutCreateInfo createPipelineLayoutCreateInfo(
			const RayTracingPipelineConfig &config,
			const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts) {
		static vk::PushConstantRange pushConstantRange;
		
		const size_t pushConstantsSize = config.getShaderProgram().getPushConstantsSize();
		pushConstantRange =vk::PushConstantRange(
				vk::ShaderStageFlagBits::eAll,
				0,
				pushConstantsSize
		);
		
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, (descriptorSetLayouts),
															  (pushConstantRange));
		
		if (pushConstantsSize == 0) {
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		}
		
		return pipelineLayoutCreateInfo;
	}
	
	RayTracingPipelineHandle
	RayTracingPipelineManager::createPipeline(const RayTracingPipelineConfig &config,
											  const DescriptorSetLayoutManager &descriptorManager,
											  BufferManager &bufferManager) {
		const auto &program = config.getShaderProgram();
		
		const auto &dynamicDispatch = getCore().getContext().getDispatchLoaderDynamic();
		
		const bool existsRayGenShader = program.existsShader(ShaderStage::RAY_GEN);
		const bool existsRayAnyHitShader = program.existsShader(ShaderStage::RAY_ANY_HIT);
		const bool existsRayClosestHitShader = program.existsShader(ShaderStage::RAY_CLOSEST_HIT);
		const bool existsRayMissShader = program.existsShader(ShaderStage::RAY_MISS);
		const bool existsRayIntersectionShader = program.existsShader(ShaderStage::RAY_INTERSECTION);
		const bool existsRayCallableShader = program.existsShader(ShaderStage::RAY_CALLABLE);
		
		if (!existsRayGenShader) {
			vkcv_log(LogLevel::ERROR, "Requires ray generation shader code");
			return {};
		}
		
		uint32_t anyHitStageIndex = VK_SHADER_UNUSED_KHR;
		uint32_t closestHitStageIndex = VK_SHADER_UNUSED_KHR;
		uint32_t intersectionStageIndex = VK_SHADER_UNUSED_KHR;
		
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(
				(existsRayGenShader? 1 : 0)
				+ (existsRayAnyHitShader? 1 : 0)
				+ (existsRayClosestHitShader? 1 : 0)
				+ (existsRayMissShader? 1 : 0)
				+ (existsRayIntersectionShader? 1 : 0)
				+ (existsRayCallableShader? 1 : 0)
		);
		
		auto destroyShaderModules = [&shaderStages, this] {
			for (auto stage : shaderStages) {
				getCore().getContext().getDevice().destroyShaderModule(stage.module);
			}
			shaderStages.clear();
		};
		
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
		shaderGroups.reserve(
				(existsRayGenShader? 1 : 0)
				+ (existsRayMissShader? 1 : 0)
				+ (existsRayCallableShader? 1 : 0)
				+ (existsRayAnyHitShader
					|| existsRayClosestHitShader
					|| existsRayIntersectionShader? 1 : 0)
		);
		
		auto addGeneralShaderGroup = [&shaderGroups](size_t index) {
			shaderGroups.emplace_back(
					vk::RayTracingShaderGroupTypeKHR::eGeneral,
					static_cast<uint32_t>(index),
					VK_SHADER_UNUSED_KHR,
					VK_SHADER_UNUSED_KHR,
					VK_SHADER_UNUSED_KHR,
					nullptr
			);
		};
		
		size_t genShaderGroupIndex = shaderGroups.capacity();
		size_t missShaderGroupIndex = genShaderGroupIndex;
		size_t hitShaderGroupIndex = genShaderGroupIndex;
		size_t callShaderGroupIndex = genShaderGroupIndex;
		
		if (existsRayGenShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_GEN,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				genShaderGroupIndex = shaderGroups.size();
				addGeneralShaderGroup(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if (existsRayAnyHitShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_ANY_HIT,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				anyHitStageIndex = static_cast<uint32_t>(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if (existsRayClosestHitShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_CLOSEST_HIT,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				closestHitStageIndex = static_cast<uint32_t>(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if (existsRayMissShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_MISS,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				missShaderGroupIndex = shaderGroups.size();
				addGeneralShaderGroup(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if (existsRayIntersectionShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_INTERSECTION,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				intersectionStageIndex = static_cast<uint32_t>(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if (existsRayCallableShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
					program,
					ShaderStage::RAY_CALLABLE,
					getCore().getContext().getDevice(),
					&createInfo
			);
			
			if (success) {
				callShaderGroupIndex = shaderGroups.size();
				addGeneralShaderGroup(shaderStages.size());
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}
		
		if ((closestHitStageIndex != VK_SHADER_UNUSED_KHR) ||
			(anyHitStageIndex != VK_SHADER_UNUSED_KHR) ||
			(intersectionStageIndex != VK_SHADER_UNUSED_KHR)) {
			hitShaderGroupIndex = shaderGroups.size();
			shaderGroups.emplace_back(
					intersectionStageIndex != VK_SHADER_UNUSED_KHR?
					vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup :
					vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
					VK_SHADER_UNUSED_KHR,
					closestHitStageIndex,
					anyHitStageIndex,
					intersectionStageIndex,
					nullptr
			);
		}
		
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(config.getDescriptorSetLayouts().size());
		for (const auto &handle : config.getDescriptorSetLayouts()) {
			descriptorSetLayouts.push_back(
					descriptorManager.getDescriptorSetLayout(handle).vulkanHandle);
		}
		
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = createPipelineLayoutCreateInfo(
				config,
				descriptorSetLayouts
		);
		
		vk::PipelineLayout pipelineLayout {};
		if (getCore().getContext().getDevice().createPipelineLayout(&pipelineLayoutCreateInfo,
																	nullptr, &pipelineLayout)
			!= vk::Result::eSuccess) {
			destroyShaderModules();
			return {};
		}
		
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
		
		vk::PhysicalDeviceProperties2 physicalProperties2;
		physicalProperties2.pNext = &rayTracingPipelineProperties;
		
		getCore().getContext().getPhysicalDevice().getProperties2(&physicalProperties2);
		
		vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo (
				vk::PipelineCreateFlags(),
				static_cast<uint32_t>(shaderStages.size()),
				shaderStages.data(),
				static_cast<uint32_t>(shaderGroups.size()),
				shaderGroups.data(),
				rayTracingPipelineProperties.maxRayRecursionDepth,
				nullptr,
				nullptr,
				nullptr,
				pipelineLayout
		);
		
		const auto pipelineCreation = getCore().getContext().getDevice().createRayTracingPipelineKHR(
				vk::DeferredOperationKHR(),
				vk::PipelineCache(),
				pipelineCreateInfo,
				nullptr,
				dynamicDispatch
		);
		
		// Clean Up
		destroyShaderModules();
		
		if (pipelineCreation.result != vk::Result::eSuccess) {
			getCore().getContext().getDevice().destroy(pipelineLayout);
			return {};
		}
		
		auto pipeline = pipelineCreation.value;
		
		const size_t shaderGroupHandlesSize = (
				rayTracingPipelineProperties.shaderGroupHandleSize * shaderGroups.size()
		);
		
		std::vector<uint8_t> shaderGroupHandleEntries;
		shaderGroupHandleEntries.resize(shaderGroupHandlesSize);
		
		if (getCore().getContext().getDevice().getRayTracingShaderGroupHandlesKHR(
				pipeline,
				0,
				static_cast<uint32_t>(shaderGroups.size()),
				static_cast<uint32_t>(shaderGroupHandlesSize),
				shaderGroupHandleEntries.data(),
				dynamicDispatch) != vk::Result::eSuccess) {
			vkcv_log(LogLevel::ERROR, "Could not retrieve shader binding table group handles.");
			
			getCore().getContext().getDevice().destroy(pipeline);
			getCore().getContext().getDevice().destroy(pipelineLayout);
			return {};
		}
		
		const size_t tableSizeAlignment = std::max(
				rayTracingPipelineProperties.shaderGroupBaseAlignment,
				rayTracingPipelineProperties.shaderGroupHandleSize
		);
		
		const size_t shaderBindingTableSize = (
				tableSizeAlignment * shaderGroups.size()
		);
		
		const BufferHandle &shaderBindingTable = getCore().createBuffer(
				BufferType::SHADER_BINDING,
				shaderBindingTableSize
		);
		
		void* mappedBindingTable = bufferManager.mapBuffer(
				shaderBindingTable,
				0,
				shaderBindingTableSize
		);
		
		for (size_t i = 0; i < shaderGroups.size(); i++) {
			memcpy(
				reinterpret_cast<uint8_t*>(mappedBindingTable)
				+ i * tableSizeAlignment,
				shaderGroupHandleEntries.data()
				+ i * rayTracingPipelineProperties.shaderGroupHandleSize,
				rayTracingPipelineProperties.shaderGroupHandleSize
			);
		}
		
		if (mappedBindingTable == nullptr) {
			getCore().getContext().getDevice().destroy(pipeline);
			getCore().getContext().getDevice().destroy(pipelineLayout);
			return {};
		}
		
		bufferManager.unmapBuffer(shaderBindingTable);
		
		const vk::DeviceAddress bufferBaseAddress = bufferManager.getBufferDeviceAddress(
				shaderBindingTable
		);
		
		vk::StridedDeviceAddressRegionKHR rayGenAddress {};
		vk::StridedDeviceAddressRegionKHR rayMissAddress {};
		vk::StridedDeviceAddressRegionKHR rayHitAddress {};
		vk::StridedDeviceAddressRegionKHR rayCallAddress {};
		
		if (genShaderGroupIndex < shaderGroups.size()) {
			rayGenAddress = vk::StridedDeviceAddressRegionKHR(
					bufferBaseAddress + genShaderGroupIndex * tableSizeAlignment,
					shaderBindingTableSize,
					shaderBindingTableSize
			);
		}
		
		if (missShaderGroupIndex < shaderGroups.size()) {
			rayMissAddress = vk::StridedDeviceAddressRegionKHR(
					bufferBaseAddress + missShaderGroupIndex * tableSizeAlignment,
					shaderBindingTableSize,
					shaderBindingTableSize
			);
		}
		
		if (hitShaderGroupIndex < shaderGroups.size()) {
			rayHitAddress = vk::StridedDeviceAddressRegionKHR(
					bufferBaseAddress + hitShaderGroupIndex * tableSizeAlignment,
					shaderBindingTableSize,
					shaderBindingTableSize
			);
		}
		
		if (callShaderGroupIndex < shaderGroups.size()) {
			rayCallAddress = vk::StridedDeviceAddressRegionKHR(
					bufferBaseAddress + callShaderGroupIndex * tableSizeAlignment,
					shaderBindingTableSize,
					shaderBindingTableSize
			);
		}
		
		return add({
			pipeline,
			pipelineLayout,
			config,
			shaderBindingTable,
			rayGenAddress,
			rayMissAddress,
			rayHitAddress,
			rayCallAddress
		});
	}
	
	vk::Pipeline RayTracingPipelineManager::getVkPipeline(const RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_handle;
	}
	
	vk::PipelineLayout RayTracingPipelineManager::getVkPipelineLayout(
			const RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_layout;
	}
	
	const RayTracingPipelineConfig &
	RayTracingPipelineManager::getPipelineConfig(const RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_config;
	}
	
	const vk::StridedDeviceAddressRegionKHR *
	RayTracingPipelineManager::getRayGenShaderBindingTableAddress(
			const vkcv::RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return &(pipeline.m_rayGenAddress);
	}
	
	const vk::StridedDeviceAddressRegionKHR *
	RayTracingPipelineManager::getMissShaderBindingTableAddress(
			const vkcv::RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return &(pipeline.m_rayMissAddress);
	}
	
	const vk::StridedDeviceAddressRegionKHR *
	RayTracingPipelineManager::getHitShaderBindingTableAddress(
			const vkcv::RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return &(pipeline.m_rayHitAddress);
	}
	
	const vk::StridedDeviceAddressRegionKHR *
	RayTracingPipelineManager::getCallShaderBindingTableAddress(
			const vkcv::RayTracingPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return &(pipeline.m_rayCallAddress);
	}
	
}
