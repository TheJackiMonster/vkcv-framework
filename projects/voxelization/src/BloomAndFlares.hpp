#pragma once
#include <vkcv/Core.hpp>
#include <glm/glm.hpp>

class BloomAndFlares{
public:
    BloomAndFlares(vkcv::Core *p_Core,
                   vk::Format colorBufferFormat,
                   uint32_t width,
                   uint32_t height);

    void execWholePipeline(const vkcv::CommandStreamHandle &cmdStream, const vkcv::ImageHandle &colorAttachment,
        const uint32_t attachmentWidth, const uint32_t attachmentHeight, const glm::vec3& cameraForward);

    void updateImageDimensions(uint32_t width, uint32_t height);

private:
    vkcv::Core *p_Core;

    vk::Format m_ColorBufferFormat;
    uint32_t m_Width;
    uint32_t m_Height;

    vkcv::SamplerHandle m_LinearSampler;
    vkcv::SamplerHandle m_RadialLutSampler;
    vkcv::Image m_Blur;
    vkcv::Image m_LensFeatures;

    vkcv::Image m_radialLut;
    vkcv::Image m_lensDirt;

    vkcv::ComputePipelineHandle              m_DownsamplePipe;
    std::vector<vkcv::DescriptorSetHandle>   m_DownsampleDescSets; // per mip desc set
    std::vector<vkcv::DescriptorSetHandle>   m_UpsampleLensFlareDescSets; // per mip desc set

    vkcv::ComputePipelineHandle              m_UpsamplePipe;
    std::vector<vkcv::DescriptorSetHandle>   m_UpsampleDescSets;   // per mip desc set

    vkcv::ComputePipelineHandle              m_LensFlarePipe;
    vkcv::DescriptorSetHandle                m_LensFlareDescSet;

    vkcv::ComputePipelineHandle              m_CompositePipe;
    vkcv::DescriptorSetHandle                m_CompositeDescSet;

    void execDownsamplePipe(const vkcv::CommandStreamHandle &cmdStream, const vkcv::ImageHandle &colorAttachment);
    void execUpsamplePipe(const vkcv::CommandStreamHandle &cmdStream);
    void execLensFeaturePipe(const vkcv::CommandStreamHandle &cmdStream);
    void execCompositePipe(const vkcv::CommandStreamHandle &cmdStream, const vkcv::ImageHandle &colorAttachment, 
        const uint32_t attachmentWidth, const uint32_t attachmentHeight, const glm::vec3& cameraForward);
};



