#pragma once

#include "Meshlet.hpp"

namespace vkcv::meshlet
{
    std::vector<uint32_t> forsythReorder(const std::vector<uint32_t> &idxBuf, const size_t vertexCount);
}
