#pragma once

#include <vulkan/vulkan.hpp>

#include "HandleManager.hpp"

#include "vkcv/Container.hpp"
#include "vkcv/PassConfig.hpp"

namespace vkcv {

	struct PassEntry {
		vk::RenderPass m_Handle;
		PassConfig m_Config;
		Vector<vk::ImageLayout> m_Layouts;
	};

	/**
	 * @brief Class to manage the creation and destruction of passes.
	 */
	class PassManager : public HandleManager<PassEntry, PassHandle> {
		friend class Core;

	private:
		[[nodiscard]] uint64_t getIdFrom(const PassHandle &handle) const override;

		[[nodiscard]] PassHandle createById(uint64_t id,
											const HandleDestroyFunction &destroy) override;

		/**
		 * Destroys and deallocates pass represented by a given
		 * pass handle id.
		 *
		 * @param id Pass handle id
		 */
		void destroyById(uint64_t id) override;

	public:
		PassManager() noexcept;

		~PassManager() noexcept override; // dtor

		[[nodiscard]] PassHandle createPass(const PassConfig &config);

		[[nodiscard]] vk::RenderPass getVkPass(const PassHandle &handle) const;

		[[nodiscard]] const PassConfig &getPassConfig(const PassHandle &handle) const;

		[[nodiscard]] const Vector<vk::ImageLayout> &
		getLayouts(const PassHandle &handle) const;
	};

} // namespace vkcv
