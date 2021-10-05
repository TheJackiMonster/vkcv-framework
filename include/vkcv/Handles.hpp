#pragma once
/**
 * @authors Artur Wasmut, Tobias Frisch
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
		Handle();
		
		explicit Handle(uint64_t id, const HandleDestroyFunction& destroy = nullptr);
		
		/**
		 * Returns the actual handle id of a handle.
		 *
		 * @return Handle id
		 */
		[[nodiscard]]
		uint64_t getId() const;
		
		/**
		 * Returns the reference counter of a handle
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
		
		explicit operator bool() const;
		bool operator!() const;
		
	};
	
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
		[[nodiscard]]
		bool isSwapchainImage() const;
		
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
