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

	VertexBufferBinding vertexBufferBinding(const BufferHandle &buffer, size_t offset = 0);

	typedef std::vector<VertexBufferBinding> VertexBufferBindings;

	/**
	 * @brief Enum class to specify the size of indexes.
	 */
	enum class IndexBitCount {
		Bit8,
		Bit16,
		Bit32
	};

	/**
	 * @brief Class to store the details of vertex data for rendering
	 */
	class VertexData {
	private:
		VertexBufferBindings m_bindings;
		BufferHandle m_indices;
		IndexBitCount m_indexBitCount;
		size_t m_count;

	public:
		/**
		 * @brief Constructor of vertex data by providing an optional vector
		 * of vertex buffer bindings.
		 *
		 * @param[in] bindings Vertex buffer bindings (optional)
		 */
		explicit VertexData(const VertexBufferBindings &bindings = {});

		VertexData(const VertexData &other) = default;
		VertexData(VertexData &&other) noexcept = default;

		~VertexData() = default;

		VertexData &operator=(const VertexData &other) = default;
		VertexData &operator=(VertexData &&other) noexcept = default;

		/**
		 * @brief Return the used vertex buffer bindings of the vertex data.
		 *
		 * @return Vertex buffer bindings
		 */
		[[nodiscard]] const VertexBufferBindings &getVertexBufferBindings() const;

		/**
		 * @brief Set the optional index buffer and its used index bit count.
		 *
		 * @param[in] indices Index buffer handle
		 * @param[in] indexBitCount Index bit count
		 */
		void setIndexBuffer(const BufferHandle &indices,
							IndexBitCount indexBitCount = IndexBitCount::Bit16);

		/**
		 * @brief Return the handle from the used index buffer of the vertex
		 * data.
		 *
		 * @return Index buffer handle
		 */
		[[nodiscard]] const BufferHandle &getIndexBuffer() const;

		/**
		 * @brief Return the index bit count of the indices used in the
		 * vertex data.
		 *
		 * @return Index bit count
		 */
		[[nodiscard]] IndexBitCount getIndexBitCount() const;

		/**
		 * @brief Set the count of elements to use by the vertex data.
		 *
		 * @param count Count of vertex elements
		 */
		void setCount(size_t count);

		/**
		 * @brief Return the count of elements in use by the vertex data.
		 *
		 * @return Count of vertex elements
		 */
		[[nodiscard]] size_t getCount() const;
	};

} // namespace vkcv
