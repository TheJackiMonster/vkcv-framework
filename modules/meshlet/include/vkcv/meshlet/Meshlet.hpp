#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <vkcv/asset/asset_loader.hpp>

namespace vkcv::meshlet {

    struct Vertex {
        glm::vec3   position;
        float       padding0;
        glm::vec3   normal;
        float       padding1;
    };

    struct Meshlet {
        uint32_t    vertexOffset;
        uint32_t    vertexCount;
        uint32_t    indexOffset;
        uint32_t    indexCount;
        glm::vec3   meanPosition;
        float       boundingSphereRadius;
    };

    struct VertexCacheReorderResult {
        /**
         * @param indexBuffer new indexBuffer
         * @param skippedIndices indices that have a spacial break
         */
        VertexCacheReorderResult(const std::vector<uint32_t> indexBuffer, const std::vector<uint32_t> skippedIndices)
                :indexBuffer(indexBuffer), skippedIndices(skippedIndices) {}

        std::vector<uint32_t> indexBuffer;
        std::vector<uint32_t>  skippedIndices;
    };

    struct MeshShaderModelData {
        std::vector<Vertex>     vertices;
        std::vector<uint32_t>   localIndices;
        std::vector<Meshlet>    meshlets;
    };

    std::vector<Vertex> convertToVertices(
            const std::vector<uint8_t>&         vertexData,
            const uint64_t                      vertexCount,
            const vkcv::asset::VertexAttribute& positionAttribute,
            const vkcv::asset::VertexAttribute& normalAttribute);

    MeshShaderModelData createMeshShaderModelData(
            const std::vector<Vertex>&      inVertices,
            const std::vector<uint32_t>&    inIndices,
            const std::vector<uint32_t>& deadEndIndices = {});

    std::vector<uint32_t> assetLoaderIndicesTo32BitIndices(
            const std::vector<uint8_t>& indexData,
            vkcv::asset::IndexType indexType);

}