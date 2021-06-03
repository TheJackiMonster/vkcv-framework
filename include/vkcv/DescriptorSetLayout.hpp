#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>

namespace vkcv{


    struct DescriptorSetLayout{
        inline DescriptorSetLayout() noexcept: sampledImages(),
                                               storageImages(),
                                               uniformBuffers(),
                                               storageBuffers(),
                                               samplers() {};
        inline DescriptorSetLayout(std::vector<uint32_t> sampledImageVec,
                                   std::vector<uint32_t> storageImageVec,
                                   std::vector<uint32_t> uniformBufferVec,
                                   std::vector<uint32_t> storageBufferVec,
                                   std::vector<uint32_t> samplerVec) noexcept:
                                   sampledImages(sampledImageVec),
                                   storageImages(storageImageVec),
                                   uniformBuffers(uniformBufferVec),
                                   storageBuffers(storageBufferVec),
                                   samplers(samplerVec) {};

        std::vector<uint32_t>		sampledImages;
        std::vector<uint32_t>		storageImages;
        std::vector<uint32_t>	    uniformBuffers;
        std::vector<uint32_t>	    storageBuffers;
        std::vector<uint32_t>		samplers;
    };

}
