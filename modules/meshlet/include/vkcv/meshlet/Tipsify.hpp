#pragma once

#include "Meshlet.hpp"
#include <algorithm>
#include <iostream>

namespace vkcv::meshlet {
    struct tipsifyResult{
        /**
         *
         * @param indexBuffer new indexBuffer
         * @param skippedIndices indices that have a spacial break
         */
        tipsifyResult(const std::vector<uint32_t> indexBuffer, const std::vector<uint32_t> skippedIndices)
        :indexBuffer(indexBuffer), skippedIndices(skippedIndices) {}

        std::vector<uint32_t> indexBuffer;
        std::vector<uint32_t>  skippedIndices;
    };

    /**
     * reorders the IndexBuffer, so all usages of vertices to triangle are as close as possible
     * @param indexBuffer32Bit current IndexBuffer
     * @param vertexCount of the mesh
     * @param cacheSize of the priority cache <br>
     * Recommended: 20. Keep the value between 5 and 50 <br>
     * low:         more random and patchy<br>
     * high:        closer vertices have higher chance -> leads to sinuous lines
     * @return new IndexBuffer that replaces the input IndexBuffer, and the indices that are skipped
     *
     * https://gfx.cs.princeton.edu/pubs/Sander_2007_%3ETR/tipsy.pdf
     * https://www.martin.st/thesis/efficient_triangle_reordering.pdf
     */
    tipsifyResult tipsifyMesh(
            const std::vector<uint32_t> &indexBuffer32Bit,
            const int vertexCount, const unsigned int cacheSize = 20);

}