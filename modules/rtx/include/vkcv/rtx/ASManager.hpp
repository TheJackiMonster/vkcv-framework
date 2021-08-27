#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::rtx {

    class ASManager {
    private:
        Core* m_core;

    public:

        /**
         * @brief TODO
         */
        ASManager(vkcv::Core *core);

        /**
         * @brief Build a Bottom Level Acceleration Structure object from given @p vertexBuffer and @p indexBuffer.
         * @param[in] vertexBuffer The vertex buffer.
         * @param[in] indexBuffer The index buffer.
         */
        void buildBLAS(Buffer<uint16_t> &vertexBuffer, Buffer<uint16_t> &indexBuffer);
    };
}