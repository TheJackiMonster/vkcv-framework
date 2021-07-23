#pragma once

#include "Meshlet.hpp"
#include <algorithm>
#include <iostream>

namespace vkcv::meshlet {
    /**
     * reorders the IndexBuffer, so all usages of vertices to triangle are as close as possible
     * @param indexBuffer32Bit current IndexBuffer
     * @param vertexCount of the mesh
     * @param cacheSize of the priority cache <br>
     * Recommended: 20. Keep the value between 5 and 50 <br>
     * low:         more random and patchy<br>
     * high:        closer vertices have higher chance -> leads to sinuous lines
     * @return new IndexBuffer that replaces the input IndexBuffer
     */
    std::vector<uint32_t> tipsifyMesh(
            const std::vector<uint32_t> &indexBuffer32Bit,
            const int vertexCount, const unsigned int cacheSize = 20);

}