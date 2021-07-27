#pragma once

#include "Meshlet.hpp"

namespace vkcv::meshlet
{
 /**
  * Reorders the index buffer, simulating a LRU cache, so that vertices are grouped together in close triangle patches
  * @param idxBuf current IndexBuffer
  * @param vertexCount of the mesh
  * @return new reordered index buffer to replace the input index buffer
  * References:
  * https://tomforsyth1000.github.io/papers/fast_vert_cache_opt.html
  * https://www.martin.st/thesis/efficient_triangle_reordering.pdf
  * https://github.com/vivkin/forsyth/blob/master/forsyth.h
  */
    std::vector<uint32_t> forsythReorder(const std::vector<uint32_t> &idxBuf, const size_t vertexCount);
}
