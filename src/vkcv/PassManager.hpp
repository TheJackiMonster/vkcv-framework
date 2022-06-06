#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include "vkcv/Handles.hpp"
#include "vkcv/PassConfig.hpp"

namespace vkcv
{
	
	/**
	 * @brief Class to manage the creation and destruction of passes.
	 */
    class PassManager
    {
    private:
    	struct Pass {
			vk::RenderPass m_Handle;
			PassConfig m_Config;
    	};
    	
        vk::Device m_Device;
        std::vector<Pass> m_Passes;
        
        void destroyPassById(uint64_t id);
        
    public:
        PassManager() = delete; // no default ctor
        explicit PassManager(vk::Device device) noexcept; // ctor
        ~PassManager() noexcept; // dtor

        PassManager(const PassManager &other) = delete; // copy-ctor
        PassManager(PassManager &&other) = delete; // move-ctor;

        PassManager & operator=(const PassManager &other) = delete; // copy-assign op
        PassManager & operator=(PassManager &&other) = delete; // move-assign op

        PassHandle createPass(const PassConfig &config);

        [[nodiscard]]
        vk::RenderPass getVkPass(const PassHandle &handle) const;
        
        [[nodiscard]]
        const PassConfig& getPassConfig(const PassHandle &handle) const;
        
    };
	
}
