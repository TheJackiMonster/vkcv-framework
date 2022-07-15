
#include "vkcv/effects/BloomAndFlaresEffect.hpp"

#include <vkcv/DrawcallRecording.hpp>
#include <vkcv/PushConstants.hpp>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/asset/asset_loader.hpp>

#include "bloomDownsample.comp.hxx"
#include "bloomFlaresComposite.comp.hxx"
#include "bloomUpsample.comp.hxx"
#include "lensFlares.comp.hxx"

namespace vkcv::effects {
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_2 = DescriptorBinding{
				2,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		descriptorBindings.insert(std::make_pair(2, binding_2));
		
		return descriptorBindings;
	}
	
	static DescriptorBindings getCompositeDescriptorBindings(bool advanced) {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_2 = DescriptorBinding{
				2,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_3 = DescriptorBinding{
				3,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		descriptorBindings.insert(std::make_pair(2, binding_2));
		descriptorBindings.insert(std::make_pair(3, binding_3));
		
		if (advanced) {
			auto binding_4 = DescriptorBinding{
					4,
					DescriptorType::IMAGE_SAMPLED,
					1,
					ShaderStage::COMPUTE,
					false
			};
			
			auto binding_5 = DescriptorBinding{
					5,
					DescriptorType::SAMPLER,
					1,
					ShaderStage::COMPUTE,
					false
			};
			
			auto binding_6 = DescriptorBinding{
					6,
					DescriptorType::IMAGE_SAMPLED,
					1,
					ShaderStage::COMPUTE,
					false
			};
			
			descriptorBindings.insert(std::make_pair(4, binding_4));
			descriptorBindings.insert(std::make_pair(5, binding_5));
			descriptorBindings.insert(std::make_pair(6, binding_6));
		}
		
		return descriptorBindings;
	}
	
	static ImageHandle loadTexture(Core &core,
								   const std::string &texturePath) {
		const auto texture = vkcv::asset::loadTexture(texturePath);
		
		Image image = core.createImage(
				vk::Format::eR8G8B8A8Unorm,
				texture.width,
				texture.height
		);
		
		image.fill(texture.data.data(), texture.data.size());
		return image.getHandle();
	}
	
	static ComputePipelineHandle compilePipeline(Core &core,
												 const std::string &shaderSource,
												 const DescriptorSetLayoutHandle &descriptorSetLayout,
												 bool advanced = false) {
		vkcv::shader::GLSLCompiler compiler;
		
		if (advanced) {
			compiler.setDefine("ADVANCED_FEATURES", "1");
		}
		
		ShaderProgram program;
		compiler.compileSource(
				vkcv::ShaderStage::COMPUTE,
				shaderSource.c_str(),
				[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
					program.addShader(shaderStage, path);
				},
				""
		);
		
		ComputePipelineHandle pipeline = core.createComputePipeline({
			program, { descriptorSetLayout }
		});
		
		return pipeline;
	}
	
	BloomAndFlaresEffect::BloomAndFlaresEffect(Core &core,
											   bool advanced) :
	Effect(core),
	m_advanced(advanced),
	
	m_downsamplePipeline(),
	m_upsamplePipeline(),
	m_lensFlaresPipeline(),
	m_compositePipeline(),
	
	m_downsampleDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_downsampleDescriptorSets({}),
	
	m_upsampleDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_upsampleDescriptorSets({}),
	m_flaresDescriptorSets({}),
	
	m_lensFlaresDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_lensFlaresDescriptorSet(m_core.createDescriptorSet(m_lensFlaresDescriptorSetLayout)),
	
	m_compositeDescriptorSetLayout(),
	m_compositeDescriptorSet(),
	
	m_blurImage(),
	m_flaresImage(),
	
	m_linearSampler(m_core.createSampler(
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerMipmapMode::LINEAR,
			vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	)),
	
	m_radialLutSampler(),
	
	m_radialLut(),
	m_lensDirt(),
	
	m_cameraDirection(),
	m_upsampleLimit(5) {
		m_downsamplePipeline = compilePipeline(
				m_core,
				BLOOMDOWNSAMPLE_COMP_SHADER,
				m_downsampleDescriptorSetLayout
		);
		
		m_upsamplePipeline = compilePipeline(
				m_core,
				BLOOMUPSAMPLE_COMP_SHADER,
				m_upsampleDescriptorSetLayout
		);
		
		m_lensFlaresPipeline = compilePipeline(
				m_core,
				LENSFLARES_COMP_SHADER,
				m_lensFlaresDescriptorSetLayout
		);
		
		m_compositeDescriptorSetLayout = m_core.createDescriptorSetLayout(
				getCompositeDescriptorBindings(m_advanced)
		);
		
		m_compositeDescriptorSet = m_core.createDescriptorSet(
				m_compositeDescriptorSetLayout
		);
		
		m_compositePipeline = compilePipeline(
				m_core,
				BLOOMFLARESCOMPOSITE_COMP_SHADER,
				m_compositeDescriptorSetLayout,
				m_advanced
		);
		
		if (m_advanced) {
			m_radialLutSampler = m_core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
			
			m_radialLut = loadTexture(m_core, "assets/RadialLUT.png");
			m_lensDirt = loadTexture(m_core, "assets/lensDirt.jpg");
		}
	}
	
	static uint32_t calcDispatchSize(float sampleSizeDim, uint32_t threadGroupWorkRegionDim) {
		return std::max<uint32_t>(
				static_cast<uint32_t>(std::ceil(
						sampleSizeDim / static_cast<float>(threadGroupWorkRegionDim)
				)),
				1
		);
	}
	
	void BloomAndFlaresEffect::recordDownsampling(const CommandStreamHandle &cmdStream,
												  const ImageHandle &input,
												  const ImageHandle &sample,
												  const std::vector<DescriptorSetHandle> &mipDescriptorSets) {
		const uint32_t sampleWidth = m_core.getImageWidth(sample);
		const uint32_t sampleHeight = m_core.getImageHeight(sample);
		
		m_core.prepareImageForSampling(cmdStream, input);
		m_core.prepareImageForStorage(cmdStream, m_blurImage);
		
		for (uint32_t mipLevel = 0; mipLevel < mipDescriptorSets.size(); mipLevel++) {
			// mip descriptor writes
			DescriptorWrites mipDownsampleWrites;
			
			if (mipLevel > 0) {
				mipDownsampleWrites.writeSampledImage(0, sample, mipLevel - 1, true);
			} else {
				mipDownsampleWrites.writeSampledImage(0, input);
			}
			
			mipDownsampleWrites.writeSampler(1, m_linearSampler);
			mipDownsampleWrites.writeStorageImage(2, sample, mipLevel);
			
			m_core.writeDescriptorSet(mipDescriptorSets[mipLevel], mipDownsampleWrites);
			
			float mipDivisor = 1.0f;
			
			for (uint32_t i = 0; i < mipLevel; i++) {
				mipDivisor *= 2.0f;
			}
			
			const auto downsampleSizeX  = static_cast<float>(sampleWidth) / mipDivisor;
			const auto downsampleSizeY  = static_cast<float>(sampleHeight) / mipDivisor;
			
			static const uint32_t threadGroupWorkRegionDim = 8;
			
			DispatchSize dispatch (
					calcDispatchSize(downsampleSizeX, threadGroupWorkRegionDim),
					calcDispatchSize(downsampleSizeY, threadGroupWorkRegionDim)
			);
			
			// mip blur dispatch
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_downsamplePipeline,
					dispatch,
					{ DescriptorSetUsage(0, mipDescriptorSets[mipLevel]) },
					PushConstants(0)
			);
			
			// image barrier between mips
			m_core.recordImageMemoryBarrier(cmdStream, sample);
		}
	}
	
	void BloomAndFlaresEffect::recordUpsampling(const CommandStreamHandle &cmdStream,
												const ImageHandle &sample,
												const std::vector<DescriptorSetHandle> &mipDescriptorSets) {
		// upsample dispatch
		m_core.prepareImageForStorage(cmdStream, sample);
		
		const uint32_t sampleWidth = m_core.getImageWidth(sample);
		const uint32_t sampleHeight = m_core.getImageHeight(sample);
		
		// upsample dispatch for each mip map
		for(uint32_t mipLevel = mipDescriptorSets.size(); mipLevel > 0; mipLevel--) {
			// mip descriptor writes
			DescriptorWrites mipUpsampleWrites;
			mipUpsampleWrites.writeSampledImage(0, sample, mipLevel, true);
			mipUpsampleWrites.writeSampler(1, m_linearSampler);
			mipUpsampleWrites.writeStorageImage(2, sample, mipLevel - 1);
			
			m_core.writeDescriptorSet(mipDescriptorSets[mipLevel - 1], mipUpsampleWrites);
			
			float mipDivisor = 1.0f;
			
			for (uint32_t i = 0; i < mipLevel - 1; i++) {
				mipDivisor *= 2.0f;
			}
			
			const auto upsampleSizeX  = static_cast<float>(sampleWidth) / mipDivisor;
			const auto upsampleSizeY  = static_cast<float>(sampleHeight) / mipDivisor;
			
			static const uint32_t threadGroupWorkRegionDim = 8;
			
			DispatchSize dispatch (
					calcDispatchSize(upsampleSizeX, threadGroupWorkRegionDim),
					calcDispatchSize(upsampleSizeY, threadGroupWorkRegionDim)
			);
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_upsamplePipeline,
					dispatch,
					{ DescriptorSetUsage(0, mipDescriptorSets[mipLevel - 1]) },
					PushConstants(0)
			);
			
			// image barrier between mips
			m_core.recordImageMemoryBarrier(cmdStream, sample);
		}
	}
	
	void BloomAndFlaresEffect::recordLensFlares(const CommandStreamHandle &cmdStream,
												uint32_t mipLevel) {
		// lens feature generation descriptor writes
		m_core.prepareImageForSampling(cmdStream, m_blurImage);
		m_core.prepareImageForStorage(cmdStream, m_flaresImage);
		
		const uint32_t flaresWidth = m_core.getImageWidth(m_flaresImage);
		const uint32_t flaresHeight = m_core.getImageHeight(m_flaresImage);
		
		DescriptorWrites lensFlaresWrites;
		lensFlaresWrites.writeSampledImage(0, m_blurImage, 0);
		lensFlaresWrites.writeSampler(1, m_linearSampler);
		lensFlaresWrites.writeStorageImage(2, m_flaresImage, mipLevel);
		
		m_core.writeDescriptorSet(m_lensFlaresDescriptorSet, lensFlaresWrites);
		
		const auto sampleSizeX = static_cast<float>(flaresWidth);
		const auto sampleSizeY = static_cast<float>(flaresHeight);
		
		float mipDivisor = 1.0f;
		
		for (uint32_t i = 0; i < mipLevel - 1; i++) {
			mipDivisor *= 2.0f;
		}
		
		static const uint32_t threadGroupWorkRegionDim = 8;
		
		// lens feature generation dispatch
		DispatchSize dispatch (
				calcDispatchSize(sampleSizeX / mipDivisor, threadGroupWorkRegionDim),
				calcDispatchSize(sampleSizeY / mipDivisor, threadGroupWorkRegionDim)
		);
		
		m_core.recordComputeDispatchToCmdStream(
				cmdStream,
				m_lensFlaresPipeline,
				dispatch,
				{ DescriptorSetUsage(0, m_lensFlaresDescriptorSet) },
				PushConstants(0)
		);
	}
	
	void BloomAndFlaresEffect::recordComposition(const CommandStreamHandle &cmdStream,
												 const ImageHandle &output) {
		const uint32_t outputWidth = m_core.getImageWidth(output);
		const uint32_t outputHeight = m_core.getImageHeight(output);
		
		m_core.prepareImageForSampling(cmdStream, m_blurImage);
		m_core.prepareImageForSampling(cmdStream, m_flaresImage);
		m_core.prepareImageForStorage(cmdStream, output);
		
		// bloom composite descriptor write
		vkcv::DescriptorWrites compositeWrites;
		
		if (m_advanced) {
			compositeWrites.writeSampledImage(
					0, m_blurImage
			).writeSampledImage(
					1, m_flaresImage
			).writeSampledImage(
					4, m_radialLut
			).writeSampledImage(
					6, m_lensDirt
			);
			
			compositeWrites.writeSampler(
					2, m_linearSampler
			).writeSampler(
					5, m_radialLutSampler
			);
			
			compositeWrites.writeStorageImage(3, output);
		} else {
			compositeWrites.writeSampledImage(
					0, m_blurImage
			).writeSampledImage(
					1, m_flaresImage
			);
			
			compositeWrites.writeSampler(2, m_linearSampler);
			
			compositeWrites.writeStorageImage(3, output);
		}
		
		m_core.writeDescriptorSet(m_compositeDescriptorSet, compositeWrites);
		
		const auto sampleWidth = static_cast<float>(outputWidth);
		const auto sampleHeight = static_cast<float>(outputHeight);
		
		static const uint32_t threadGroupWorkRegionDim = 8;
		
		DispatchSize dispatch (
				calcDispatchSize(sampleWidth, threadGroupWorkRegionDim),
				calcDispatchSize(sampleHeight, threadGroupWorkRegionDim)
		);
		
		PushConstants pushConstants = vkcv::pushConstants<glm::vec3>();
		pushConstants.appendDrawcall(m_cameraDirection);
		
		// bloom composite dispatch
		m_core.recordComputeDispatchToCmdStream(
				cmdStream,
				m_compositePipeline,
				dispatch,
				{ DescriptorSetUsage(0, m_compositeDescriptorSet) },
				m_advanced? pushConstants : PushConstants(0)
		);
	}
	
	void BloomAndFlaresEffect::recordEffect(const CommandStreamHandle &cmdStream,
											const ImageHandle &input,
											const ImageHandle &output) {
		m_core.recordBeginDebugLabel(cmdStream, "vkcv::post_processing::BloomAndFlaresEffect", {
				0.0f, 1.0f, 1.0f, 1.0f
		});
		
		const auto halfWidth = static_cast<uint32_t>(std::ceil(
				static_cast<float>(m_core.getImageWidth(output)) * 0.5f
		));
		
		const auto halfHeight = static_cast<uint32_t>(std::ceil(
				static_cast<float>(m_core.getImageHeight(output)) * 0.5f
		));
		
		if ((!m_blurImage) ||
			(halfWidth != m_core.getImageWidth(m_blurImage)) ||
			(halfHeight != m_core.getImageHeight(m_blurImage))) {
			m_blurImage = m_core.createImage(
					m_core.getImageFormat(output),
					halfWidth,
					halfHeight,
					1,
					true,
					true
			).getHandle();
			
			m_downsampleDescriptorSets.clear();
			m_upsampleDescriptorSets.clear();
			
			const uint32_t mipLevels = m_core.getImageMipLevels(m_blurImage);
			
			for (uint32_t i = 0; i < mipLevels; i++) {
				m_downsampleDescriptorSets.push_back(m_core.createDescriptorSet(
						m_downsampleDescriptorSetLayout
				));
			}
			
			for (uint32_t i = 0; i < std::min<uint32_t>(m_upsampleLimit, mipLevels); i++) {
				m_upsampleDescriptorSets.push_back(m_core.createDescriptorSet(
						m_upsampleDescriptorSetLayout
				));
			}
		}
		
		if ((!m_flaresImage) ||
			(halfWidth != m_core.getImageWidth(m_flaresImage)) ||
			(halfHeight != m_core.getImageHeight(m_flaresImage))) {
			m_flaresImage = m_core.createImage(
					m_core.getImageFormat(output),
					halfWidth,
					halfHeight,
					1,
					true,
					true
			).getHandle();
			
			m_flaresDescriptorSets.clear();
			
			const uint32_t mipLevels = m_core.getImageMipLevels(m_flaresImage);
			
			for (uint32_t i = 0; i < std::min<uint32_t>(2, mipLevels); i++) {
				m_flaresDescriptorSets.push_back(m_core.createDescriptorSet(
						m_upsampleDescriptorSetLayout
				));
			}
		}
		
		recordDownsampling(cmdStream, input, m_blurImage, m_downsampleDescriptorSets);
		recordUpsampling(cmdStream, m_blurImage, m_upsampleDescriptorSets);
		recordLensFlares(cmdStream, m_flaresDescriptorSets.size());
		recordUpsampling(cmdStream, m_flaresImage, m_flaresDescriptorSets);
		recordComposition(cmdStream, output);
		
		m_core.recordEndDebugLabel(cmdStream);
	}
	
	void BloomAndFlaresEffect::updateCameraDirection(const camera::Camera &camera) {
		m_cameraDirection = camera.getFront();
	}
	
	void BloomAndFlaresEffect::setUpsamplingLimit(uint32_t limit) {
		m_upsampleLimit = limit;
	}
	
}
