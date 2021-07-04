#include "BloomAndFlares.hpp"
#include <vkcv/shader/GLSLCompiler.hpp>

BloomAndFlares::BloomAndFlares(
        vkcv::Core *p_Core,
        vk::Format colorBufferFormat,
        uint32_t width,
        uint32_t height) :

        p_Core(p_Core),
        m_ColorBufferFormat(colorBufferFormat),
        m_Width(width),
        m_Height(height),
        m_LinearSampler(p_Core->createSampler(vkcv::SamplerFilterType::LINEAR,
                                              vkcv::SamplerFilterType::LINEAR,
                                              vkcv::SamplerMipmapMode::LINEAR,
                                              vkcv::SamplerAddressMode::CLAMP_TO_EDGE)),
        m_Blur(p_Core->createImage(colorBufferFormat, width, height, 1, true, true, false)),
        m_LensFeatures(p_Core->createImage(colorBufferFormat, width, height, 1, false, true, false))
{
    vkcv::shader::GLSLCompiler compiler;

    // DOWNSAMPLE
    vkcv::ShaderProgram dsProg;
    compiler.compile(vk::ShaderStageFlagBits::eCompute,
                     "shaders/bloom/downsample.comp",
                     [&](vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& path)
                     {
                         dsProg.addShader(shaderStage, path);
                     });
    for(uint32_t mipLevel = 0; mipLevel < m_Blur.getMipCount(); mipLevel++)
    {
		m_DownsampleDescSets.push_back(
                p_Core->createDescriptorSet(dsProg.getReflectedDescriptors().at(0)));
    }
    m_DownsamplePipe = p_Core->createComputePipeline(
            dsProg, { p_Core->getDescriptorSet(m_DownsampleDescSets[0]).layout });

    // UPSAMPLE
    vkcv::ShaderProgram usProg;
    compiler.compile(vk::ShaderStageFlagBits::eCompute,
                     "shaders/bloom/upsample.comp",
                     [&](vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& path)
                     {
                         usProg.addShader(shaderStage, path);
                     });
    for(uint32_t mipLevel = 0; mipLevel < m_Blur.getMipCount(); mipLevel++)
    {
        m_UpsampleDescSets.push_back(
                p_Core->createDescriptorSet(usProg.getReflectedDescriptors().at(0)));
    }
    m_UpsamplePipe = p_Core->createComputePipeline(
            usProg, { p_Core->getDescriptorSet(m_UpsampleDescSets[0]).layout });

    // LENS FEATURES
    vkcv::ShaderProgram lensProg;
    compiler.compile(vk::ShaderStageFlagBits::eCompute,
                     "shaders/bloom/lensFlares.comp",
                     [&](vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& path)
                     {
                         lensProg.addShader(shaderStage, path);
                     });
    m_LensFlareDescSet = p_Core->createDescriptorSet(lensProg.getReflectedDescriptors().at(0));
    m_LensFlarePipe = p_Core->createComputePipeline(
            lensProg, { p_Core->getDescriptorSet(m_LensFlareDescSet).layout });

    // COMPOSITE
    vkcv::ShaderProgram compProg;
    compiler.compile(vk::ShaderStageFlagBits::eCompute,
                     "shaders/bloom/composite.comp",
                     [&](vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& path)
                     {
                         compProg.addShader(shaderStage, path);
                     });
    m_CompositeDescSet = p_Core->createDescriptorSet(compProg.getReflectedDescriptors().at(0));
    m_CompositePipe = p_Core->createComputePipeline(
            compProg, { p_Core->getDescriptorSet(m_CompositeDescSet).layout });
}

void BloomAndFlares::execDownsamplePipe(const vkcv::CommandStreamHandle &cmdStream,
                                        const vkcv::ImageHandle &colorAttachment)
{
    auto dispatchCountX  = static_cast<float>(m_Width)  / 8.0f;
    auto dispatchCountY = static_cast<float>(m_Height) / 8.0f;
    // blur dispatch
    uint32_t initialDispatchCount[3] = {
            static_cast<uint32_t>(glm::ceil(dispatchCountX)),
            static_cast<uint32_t>(glm::ceil(dispatchCountY)),
            1
    };

    // downsample dispatch of original color attachment
    p_Core->prepareImageForSampling(cmdStream, colorAttachment);
    p_Core->prepareImageForStorage(cmdStream, m_Blur.getHandle());

    vkcv::DescriptorWrites initialDownsampleWrites;
    initialDownsampleWrites.sampledImageWrites = {vkcv::SampledImageDescriptorWrite(0, colorAttachment)};
    initialDownsampleWrites.samplerWrites      = {vkcv::SamplerDescriptorWrite(1, m_LinearSampler)};
    initialDownsampleWrites.storageImageWrites = {vkcv::StorageImageDescriptorWrite(2, m_Blur.getHandle(), 0) };
    p_Core->writeDescriptorSet(m_DownsampleDescSets[0], initialDownsampleWrites);

    p_Core->recordComputeDispatchToCmdStream(
            cmdStream,
            m_DownsamplePipe,
            initialDispatchCount,
            {vkcv::DescriptorSetUsage(0, p_Core->getDescriptorSet(m_DownsampleDescSets[0]).vulkanHandle)},
            vkcv::PushConstantData(nullptr, 0));

    // downsample dispatches of blur buffer's mip maps
    float mipDispatchCountX = dispatchCountX;
    float mipDispatchCountY = dispatchCountY;
    for(uint32_t mipLevel = 1; mipLevel < std::min((uint32_t)m_DownsampleDescSets.size(), m_Blur.getMipCount()); mipLevel++)
    {
        // mip descriptor writes
        vkcv::DescriptorWrites mipDescriptorWrites;
        mipDescriptorWrites.sampledImageWrites = {vkcv::SampledImageDescriptorWrite(0, m_Blur.getHandle(), mipLevel - 1, true)};
        mipDescriptorWrites.samplerWrites      = {vkcv::SamplerDescriptorWrite(1, m_LinearSampler)};
        mipDescriptorWrites.storageImageWrites = {vkcv::StorageImageDescriptorWrite(2, m_Blur.getHandle(), mipLevel) };
        p_Core->writeDescriptorSet(m_DownsampleDescSets[mipLevel], mipDescriptorWrites);

        // mip dispatch calculation
        mipDispatchCountX  /= 2.0f;
        mipDispatchCountY /= 2.0f;

        uint32_t mipDispatchCount[3] = {
                static_cast<uint32_t>(glm::ceil(mipDispatchCountX)),
                static_cast<uint32_t>(glm::ceil(mipDispatchCountY)),
                1
        };

        if(mipDispatchCount[0] == 0)
            mipDispatchCount[0] = 1;
        if(mipDispatchCount[1] == 0)
            mipDispatchCount[1] = 1;

        // mip blur dispatch
        p_Core->recordComputeDispatchToCmdStream(
                cmdStream,
                m_DownsamplePipe,
                mipDispatchCount,
                {vkcv::DescriptorSetUsage(0, p_Core->getDescriptorSet(m_DownsampleDescSets[mipLevel]).vulkanHandle)},
                vkcv::PushConstantData(nullptr, 0));

        // image barrier between mips
        p_Core->recordImageMemoryBarrier(cmdStream, m_Blur.getHandle());
    }
}

void BloomAndFlares::execUpsamplePipe(const vkcv::CommandStreamHandle &cmdStream)
{
    // upsample dispatch
    p_Core->prepareImageForStorage(cmdStream, m_Blur.getHandle());

    const uint32_t upsampleMipLevels = std::min(
    		static_cast<uint32_t>(m_UpsampleDescSets.size() - 1),
    		static_cast<uint32_t>(3)
	);

    // upsample dispatch for each mip map
    for(uint32_t mipLevel = upsampleMipLevels; mipLevel > 0; mipLevel--)
    {
        // mip descriptor writes
        vkcv::DescriptorWrites mipUpsampleWrites;
        mipUpsampleWrites.sampledImageWrites = {vkcv::SampledImageDescriptorWrite(0, m_Blur.getHandle(), mipLevel, true)};
        mipUpsampleWrites.samplerWrites      = {vkcv::SamplerDescriptorWrite(1, m_LinearSampler)};
        mipUpsampleWrites.storageImageWrites = {vkcv::StorageImageDescriptorWrite(2, m_Blur.getHandle(), mipLevel - 1) };
        p_Core->writeDescriptorSet(m_UpsampleDescSets[mipLevel], mipUpsampleWrites);

        auto mipDivisor = glm::pow(2.0f, static_cast<float>(mipLevel) - 1.0f);

        auto upsampleDispatchX  = static_cast<float>(m_Width) / mipDivisor;
        auto upsampleDispatchY = static_cast<float>(m_Height) / mipDivisor;
        upsampleDispatchX /= 8.0f;
        upsampleDispatchY /= 8.0f;

        const uint32_t upsampleDispatchCount[3] = {
                static_cast<uint32_t>(glm::ceil(upsampleDispatchX)),
                static_cast<uint32_t>(glm::ceil(upsampleDispatchY)),
                1
        };

        p_Core->recordComputeDispatchToCmdStream(
                cmdStream,
                m_UpsamplePipe,
                upsampleDispatchCount,
                {vkcv::DescriptorSetUsage(0, p_Core->getDescriptorSet(m_UpsampleDescSets[mipLevel]).vulkanHandle)},
                vkcv::PushConstantData(nullptr, 0)
        );
        // image barrier between mips
        p_Core->recordImageMemoryBarrier(cmdStream, m_Blur.getHandle());
    }
}

void BloomAndFlares::execLensFeaturePipe(const vkcv::CommandStreamHandle &cmdStream)
{
    // lens feature generation descriptor writes
    p_Core->prepareImageForSampling(cmdStream, m_Blur.getHandle());
    p_Core->prepareImageForStorage(cmdStream, m_LensFeatures.getHandle());

    vkcv::DescriptorWrites lensFeatureWrites;
    lensFeatureWrites.sampledImageWrites = {vkcv::SampledImageDescriptorWrite(0, m_Blur.getHandle(), 0)};
    lensFeatureWrites.samplerWrites = {vkcv::SamplerDescriptorWrite(1, m_LinearSampler)};
    lensFeatureWrites.storageImageWrites = {vkcv::StorageImageDescriptorWrite(2, m_LensFeatures.getHandle(), 0)};
    p_Core->writeDescriptorSet(m_LensFlareDescSet, lensFeatureWrites);

    auto dispatchCountX  = static_cast<float>(m_Width)  / 8.0f;
    auto dispatchCountY = static_cast<float>(m_Height) / 8.0f;
    // lens feature generation dispatch
    uint32_t lensFeatureDispatchCount[3] = {
            static_cast<uint32_t>(glm::ceil(dispatchCountX)),
            static_cast<uint32_t>(glm::ceil(dispatchCountY)),
            1
    };
    p_Core->recordComputeDispatchToCmdStream(
            cmdStream,
            m_LensFlarePipe,
            lensFeatureDispatchCount,
            {vkcv::DescriptorSetUsage(0, p_Core->getDescriptorSet(m_LensFlareDescSet).vulkanHandle)},
            vkcv::PushConstantData(nullptr, 0));
}

void BloomAndFlares::execCompositePipe(const vkcv::CommandStreamHandle &cmdStream,
                                       const vkcv::ImageHandle &colorAttachment)
{
    p_Core->prepareImageForSampling(cmdStream, m_Blur.getHandle());
    p_Core->prepareImageForSampling(cmdStream, m_LensFeatures.getHandle());
    p_Core->prepareImageForStorage(cmdStream, colorAttachment);

    // bloom composite descriptor write
    vkcv::DescriptorWrites compositeWrites;
    compositeWrites.sampledImageWrites = {vkcv::SampledImageDescriptorWrite(0, m_Blur.getHandle()),
                                          vkcv::SampledImageDescriptorWrite(1, m_LensFeatures.getHandle())};
    compositeWrites.samplerWrites = {vkcv::SamplerDescriptorWrite(2, m_LinearSampler)};
    compositeWrites.storageImageWrites = {vkcv::StorageImageDescriptorWrite(3, colorAttachment)};
    p_Core->writeDescriptorSet(m_CompositeDescSet, compositeWrites);

    float dispatchCountX = static_cast<float>(m_Width)  / 8.0f;
    float dispatchCountY = static_cast<float>(m_Height) / 8.0f;

    uint32_t compositeDispatchCount[3] = {
            static_cast<uint32_t>(glm::ceil(dispatchCountX)),
            static_cast<uint32_t>(glm::ceil(dispatchCountY)),
            1
    };

    // bloom composite dispatch
    p_Core->recordComputeDispatchToCmdStream(
            cmdStream,
            m_CompositePipe,
            compositeDispatchCount,
            {vkcv::DescriptorSetUsage(0, p_Core->getDescriptorSet(m_CompositeDescSet).vulkanHandle)},
            vkcv::PushConstantData(nullptr, 0));
}

void BloomAndFlares::execWholePipeline(const vkcv::CommandStreamHandle &cmdStream,
                                       const vkcv::ImageHandle &colorAttachment)
{
    execDownsamplePipe(cmdStream, colorAttachment);
    execUpsamplePipe(cmdStream);
    execLensFeaturePipe(cmdStream);
    execCompositePipe(cmdStream, colorAttachment);
}

void BloomAndFlares::updateImageDimensions(uint32_t width, uint32_t height)
{
    m_Width  = width;
    m_Height = height;

    p_Core->getContext().getDevice().waitIdle();
    m_Blur = p_Core->createImage(m_ColorBufferFormat, m_Width, m_Height, 1, true, true, false);
    m_LensFeatures = p_Core->createImage(m_ColorBufferFormat, m_Width, m_Height, 1, false, true, false);
}


