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
         * @brief TODO
         * TODO: kill AS, kill buffers, free memory of buffers
         */
        ~ASManager() {};

        /**
         * @brief Build a Bottom Level Acceleration Structure object from given @p vertices and @p indices.
         * @param[in] vertices The vertex data of type uint8_t.
         * @param[in] indices The index data of type uint8_t.
         */
        void buildBLAS(std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices);

        /**
         * @brief TODO
         * @param data
         * @return
         */
        vk::Buffer makeBuffer(std::vector<uint8_t> &data);
    };
}