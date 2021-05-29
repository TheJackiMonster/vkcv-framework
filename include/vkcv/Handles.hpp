#pragma once
/**
 * @authors Artur Wasmut
 * @file src/vkcv/Handles.cpp
 * @brief Central header file for all possible handles that the framework will hand out.
 */

#include <iostream>

namespace vkcv
{
	
	class Handle {
		friend std::ostream& operator << (std::ostream& out, const Handle& handle);
		
	private:
		uint64_t m_id;
	
	protected:
		Handle();
		
		explicit Handle(uint64_t id);
		
		[[nodiscard]]
		uint64_t getId() const;
	
	public:
		virtual ~Handle() = default;
		
		Handle(const Handle& other) = default;
		Handle(Handle&& other) = default;
		
		Handle& operator=(const Handle& other) = default;
		Handle& operator=(Handle&& other) = default;
		
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
	
	class PipelineHandle : public Handle {
		friend class PipelineManager;
	private:
		using Handle::Handle;
	};
	
	class ResourcesHandle : public Handle {
		friend class DescriptorManager;
	private:
		using Handle::Handle;
	};
	
	class SamplerHandle : public Handle {
		friend class SamplerManager;
	private:
		using Handle::Handle;
	};
	
}
