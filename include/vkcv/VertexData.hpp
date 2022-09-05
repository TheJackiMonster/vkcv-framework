#pragma once
/**
 * @authors Sebastian Gaida, Alexander Gauggel, Artur Wasmut, Tobias Frisch
 * @file vkcv/VertexData.hpp
 * @brief Types to configure vertex data for drawcalls.
 */

#include <vector>

#include "Handles.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to store details about a vertex buffer binding.
	 */
	struct VertexBufferBinding {
		BufferHandle buffer;
		size_t offset;
	};
	
	VertexBufferBinding vertexBufferBinding(const BufferHandle &buffer,
											size_t offset = 0);
	
	typedef std::vector<VertexBufferBinding> VertexBufferBindings;
	
	/**
	 * @brief Enum class to specify the size of indexes.
	 */
	enum class IndexBitCount {
		Bit8,
		Bit16,
		Bit32
	};
	
	class VertexData {
	private:
		VertexBufferBindings m_bindings;
		BufferHandle m_indices;
		IndexBitCount m_indexBitCount;
		size_t m_count;
	
	public:
		explicit VertexData(const VertexBufferBindings &bindings = {});
		
		VertexData(const VertexData& other) = default;
		VertexData(VertexData&& other) noexcept = default;
		
		~VertexData() = default;
		
		VertexData& operator=(const VertexData& other) = default;
		VertexData& operator=(VertexData&& other) noexcept = default;
		
		[[nodiscard]]
		const VertexBufferBindings& getVertexBufferBindings() const;
		
		void setIndexBuffer(const BufferHandle& indices,
							IndexBitCount indexBitCount = IndexBitCount::Bit16);
		
		[[nodiscard]]
		const BufferHandle& getIndexBuffer() const;
		
		[[nodiscard]]
		IndexBitCount getIndexBitCount() const;
		
		void setCount(size_t count);
		
		[[nodiscard]]
		size_t getCount() const;
	
	};

}
