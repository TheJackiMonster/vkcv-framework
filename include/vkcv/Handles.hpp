#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel, Artur Wasmut, Sebastian Gaida, Mark Mints
 * @file vkcv/Handles.hpp
 * @brief Central header file for all possible handles that the framework will hand out.
 */

#include <iostream>

#include "Event.hpp"

namespace vkcv
{
	
	typedef typename event_function<uint64_t>::type HandleDestroyFunction;
	
	class Handle {
		friend std::ostream& operator << (std::ostream& out, const Handle& handle);
		
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
		explicit Handle(uint64_t id, const HandleDestroyFunction& destroy = nullptr);
		
		/**
		 * @brief Returns the actual handle id of a handle.
		 *
		 * @return Handle id
		 */
		[[nodiscard]]
		uint64_t getId() const;
		
		/**
		 * @brief Returns the reference counter of a handle
		 *
		 * @return Reference counter
		 */
		[[nodiscard]]
		uint64_t getRC() const;
	
	public:
		virtual ~Handle();
		
		Handle(const Handle& other);
		Handle(Handle&& other) noexcept;
		
		Handle& operator=(const Handle& other);
		Handle& operator=(Handle&& other) noexcept;
		
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
	std::ostream& operator << (std::ostream& out, const Handle& handle);
	
    // Handle returned for any buffer created with the core/context objects
    class BufferHandle : public Handle {
    	friend class BufferManager;
	private:
		using Handle::Handle;
    };
	
	class PassHandle : public Handle {
		friend class PassManager;
	private:
		using Handle::Handle;
	};
	
	class GraphicsPipelineHandle : public Handle {
		friend class GraphicsPipelineManager;
	private:
		using Handle::Handle;
	};

    class ComputePipelineHandle : public Handle {
        friend class ComputePipelineManager;
    private:
        using Handle::Handle;
    };
	
	class DescriptorSetHandle : public Handle {
		friend class DescriptorManager;
	private:
		using Handle::Handle;
	};

	class DescriptorSetLayoutHandle : public Handle {
	    friend class DescriptorManager;
	private:
	    using Handle::Handle;
	};
	
	class SamplerHandle : public Handle {
		friend class SamplerManager;
	private:
		using Handle::Handle;
	};

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
		[[nodiscard]]
		bool isSwapchainImage() const;
		
		/**
		 * @brief Creates a valid image handle to represent a swapchain image
		 * using an optional destroy callback.
		 *
		 * @param[in] destroy Destroy callback (optional)
		 * @return New swapchain image handle
		 */
		static ImageHandle createSwapchainImageHandle(const HandleDestroyFunction& destroy = nullptr);
		
	};

	class WindowHandle : public Handle {
		friend class WindowManager;
	private:
		using Handle::Handle;
	};

	class SwapchainHandle : public Handle {
		friend class SwapchainManager;
	private:
		using Handle::Handle;
	};

    class CommandStreamHandle : public Handle {
        friend class CommandStreamManager;
    private:
        using Handle::Handle;
    };
	
}
