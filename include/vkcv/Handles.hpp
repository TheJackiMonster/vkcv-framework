#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel, Artur Wasmut, Sebastian Gaida, Mark Mints
 * @file vkcv/Handles.hpp
 * @brief Central header file for all possible handles that the framework will hand out.
 */

#include <iostream>

#include "Event.hpp"

namespace vkcv {

	/**
	 * @brief Function to be called when a handles resources can be destroyed.
	 */
	typedef typename event_function<uint64_t>::type HandleDestroyFunction;

	/**
	 * @brief Class for general memory management via handles.
	 */
	class Handle {
		friend std::ostream &operator<<(std::ostream &out, const Handle &handle);

	private:
		uint64_t m_id;
		uint64_t* m_rc;

		HandleDestroyFunction m_destroy;

	protected:
		/**
		 * @brief Constructor of an invalid handle
		 */
		Handle();

		/**
		 * @brief Constructor of a valid handle with an
		 * unique id and an optional destroy callback.
		 *
		 * @param[in] id Unique handle id
		 * @param[in] destroy Destroy callback (optional)
		 */
		explicit Handle(uint64_t id, const HandleDestroyFunction &destroy = nullptr);

		/**
		 * @brief Returns the actual handle id of a handle.
		 *
		 * @return Handle id
		 */
		[[nodiscard]] uint64_t getId() const;

		/**
		 * @brief Returns the reference counter of a handle
		 *
		 * @return Reference counter
		 */
		[[nodiscard]] uint64_t getRC() const;

	public:
		virtual ~Handle();

		Handle(const Handle &other);
		Handle(Handle &&other) noexcept;

		Handle &operator=(const Handle &other);
		Handle &operator=(Handle &&other) noexcept;

		/**
		 * @brief Returns whether a handle is valid to use.
		 *
		 * @return True, if the handle is valid, else false.
		 */
		explicit operator bool() const;

		/**
		 * @brief Returns whether a handle is invalid to use.
		 *
		 * @return True, if the handle is invalid, else false.
		 */
		bool operator!() const;
	};

	/**
	 * @brief Stream operator to print a handle into an output
	 * stream.
	 *
	 * @param[out] out Output stream
	 * @param[in] handle
	 * @return Output stream after printing
	 */
	std::ostream &operator<<(std::ostream &out, const Handle &handle);

	/**
	 * @brief Handle class for buffers.
	 */
	class BufferHandle : public Handle {
		friend class BufferManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for render passes.
	 */
	class PassHandle : public Handle {
		friend class PassManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for graphics pipelines.
	 */
	class GraphicsPipelineHandle : public Handle {
		friend class GraphicsPipelineManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for compute pipelines.
	 */
	class ComputePipelineHandle : public Handle {
		friend class ComputePipelineManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for descriptor set layouts.
	 */
	class DescriptorSetLayoutHandle : public Handle {
		friend class DescriptorSetLayoutManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for descriptor sets.
	 */
	class DescriptorSetHandle : public Handle {
		friend class DescriptorSetManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for samplers.
	 */
	class SamplerHandle : public Handle {
		friend class SamplerManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for images.
	 */
	class ImageHandle : public Handle {
		friend class ImageManager;

	private:
		using Handle::Handle;

	public:
		/**
		 * @brief Returns whether the handle represents an swapchain image.
		 *
		 * @return True, if the handle represents a swapchain image, else false.
		 */
		[[nodiscard]] bool isSwapchainImage() const;

		/**
		 * @brief Creates a valid image handle to represent a swapchain image
		 * using an optional destroy callback.
		 *
		 * @param[in] destroy Destroy callback (optional)
		 * @return New swapchain image handle
		 */
		static ImageHandle
		createSwapchainImageHandle(const HandleDestroyFunction &destroy = nullptr);
	};

	/**
	 * @brief Handle class for windows.
	 */
	class WindowHandle : public Handle {
		friend class WindowManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for swapchains.
	 */
	class SwapchainHandle : public Handle {
		friend class SwapchainManager;

	private:
		using Handle::Handle;
	};

	/**
	 * @brief Handle class for command streams.
	 */
	class CommandStreamHandle : public Handle {
		friend class CommandStreamManager;

	private:
		using Handle::Handle;
	};

} // namespace vkcv
