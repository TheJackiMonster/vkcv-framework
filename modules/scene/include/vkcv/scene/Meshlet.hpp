#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>

namespace vkcv::scene {

    struct Vertex {
        glm::vec3   position;
        float       padding0;
        glm::vec3   normal;
        float       padding1;
    };

    struct Meshlet {
        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t indexOffset;
        uint32_t indexCount;
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
            const vkcv::asset::VertexAttribute& normalAttribute) {

        assert(positionAttribute.type   == vkcv::asset::PrimitiveType::POSITION);
        assert(normalAttribute.type     == vkcv::asset::PrimitiveType::NORMAL);

        std::vector<Vertex> vertices;
        vertices.reserve(vertexCount);

        const size_t positionStepSize   = positionAttribute.stride == 0 ? sizeof(glm::vec3) : positionAttribute.stride;
        const size_t normalStepSize     = normalAttribute.stride   == 0 ? sizeof(glm::vec3) : normalAttribute.stride;

        for (int i = 0; i < vertexCount; i++) {
            Vertex v;

            const size_t positionOffset = positionAttribute.offset  + positionStepSize  * i;
            const size_t normalOffset   = normalAttribute.offset    + normalStepSize    * i;

            v.position  = *reinterpret_cast<const glm::vec3*>(&(vertexData[positionOffset]));
            v.normal    = *reinterpret_cast<const glm::vec3*>(&(vertexData[normalOffset]));
            vertices.push_back(v);
        }
        return vertices;
    }

    MeshShaderModelData createMeshShaderModelData(
            const std::vector<Vertex>&      inVertices,
            const std::vector<uint32_t>&    inIndices) {

        MeshShaderModelData data;
        size_t currentIndex = 0;

        const size_t maxVerticesPerMeshlet = 64;
        const size_t maxIndicesPerMeshlet  = 126 * 3;

        bool indicesAreLeft = true;

        while (indicesAreLeft) {
            Meshlet meshlet;

            meshlet.indexCount  = 0;
            meshlet.vertexCount = 0;

            meshlet.indexOffset  = data.localIndices.size();
            meshlet.vertexOffset = data.vertices.size();

            std::map<uint32_t, uint32_t> globalToLocalIndexMap;
            std::vector<uint32_t> globalIndicesOrdered;

            while (true) {

                indicesAreLeft = currentIndex + 1 <= inIndices.size();
                if (!indicesAreLeft) {
                    break;
                }

                bool enoughSpaceForIndices = meshlet.indexCount + 3 < maxIndicesPerMeshlet;
                if (!enoughSpaceForIndices) {
                    break;
                }

                size_t vertexCountToAdd = 0;
                for (int i = 0; i < 3; i++) {
                    const uint32_t globalIndex = inIndices[currentIndex + i];
                    const bool containsVertex  = globalToLocalIndexMap.find(globalIndex) != globalToLocalIndexMap.end();
                    if (!containsVertex) {
                        vertexCountToAdd++;
                    }
                }

                bool enoughSpaceForVertices = meshlet.vertexCount + vertexCountToAdd < maxVerticesPerMeshlet;
                if (!enoughSpaceForVertices) {
                    break;
                }

                for (int i = 0; i < 3; i++) {
                    const uint32_t globalIndex = inIndices[currentIndex + i];

                    uint32_t localIndex;
                    const bool indexAlreadyExists = globalToLocalIndexMap.find(globalIndex) != globalToLocalIndexMap.end();
                    if (indexAlreadyExists) {
                        localIndex = globalToLocalIndexMap[globalIndex];
                    }
                    else {
                        localIndex = globalToLocalIndexMap.size();
                        globalToLocalIndexMap[globalIndex] = localIndex;
                        globalIndicesOrdered.push_back(globalIndex);
                    }

                    data.localIndices.push_back(localIndex);
                }

                meshlet.indexCount  += 3;
                currentIndex        += 3;
                meshlet.vertexCount += vertexCountToAdd;
            }

            for (const uint32_t globalIndex : globalIndicesOrdered) {
                const Vertex v = inVertices[globalIndex];
                data.vertices.push_back(v);
            }

            data.meshlets.push_back(meshlet);
        }

        return data;
    }

    std::vector<uint32_t> assetLoaderIndicesTo32BitIndices(const std::vector<uint8_t>& indexData, vkcv::asset::IndexType indexType) {
        std::vector<uint32_t> indices;
        if (indexType == vkcv::asset::IndexType::UINT16) {
            for (int i = 0; i < indexData.size(); i += 2) {
                const uint16_t index16Bit = *reinterpret_cast<const uint16_t*>(&(indexData[i]));
                const uint32_t index32Bit = static_cast<uint32_t>(index16Bit);
                indices.push_back(index32Bit);
            }
        }
        else if (indexType == vkcv::asset::IndexType::UINT32) {
            for (int i = 0; i < indexData.size(); i += 4) {
                const uint32_t index32Bit = *reinterpret_cast<const uint32_t*>(&(indexData[i]));
                indices.push_back(index32Bit);
            }
        }
        else {
            vkcv_log(vkcv::LogLevel::ERROR, "Unsupported index type");
        }
        return indices;
    }

}