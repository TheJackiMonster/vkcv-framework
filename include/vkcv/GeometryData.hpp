#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/GeometryData.hpp
 * @brief Types to configure geometry data for acceleration structure building.
 */

#include "VertexData.hpp"

namespace vkcv {
	
	/**
	 * @brief Enum class to specify the format of vertex in geometry.
	 */
	enum class GeometryVertexType {
		POSITION_FLOAT3,
		UNDEFINED
	};
	
	/**
	 * @brief Class to store the details of geometry data for acceleration structure building
	 */
	class GeometryData {
	private:
		VertexBufferBinding m_vertexBinding;
		size_t m_maxVertexIndex;
		GeometryVertexType m_vertexType;
		BufferHandle m_indices;
		IndexBitCount m_indexBitCount;
		size_t m_count;
	
	public:
		/**
		 * @brief Default constructor of invalid geometry data.
		 */
		GeometryData();
		
		/**
		 * @brief Constructor of geometry data by providing an vertex buffer binding,
		 * the used stride for vertex elements and its geometry vertex type.
		 *
		 * @param[in] binding Geometry buffer binding
		 * @param[in] stride Vertex element stride
		 * @param[in] geometryVertexType Geometry vertex type
		 */
		explicit GeometryData(const VertexBufferBinding &binding,
							  size_t maxVertexIndex = 0,
							  GeometryVertexType geometryVertexType =
									  GeometryVertexType::POSITION_FLOAT3);
		
		GeometryData(const GeometryData &other) = default;
		GeometryData(GeometryData &&other) noexcept = default;
		
		~GeometryData() = default;
		
		GeometryData &operator=(const GeometryData &other) = default;
		GeometryData &operator=(GeometryData &&other) noexcept = default;
		
		/**
		 * @brief Return whether the geometry is valid to use.
		 *
		 * @return True if the geometry data is valid, otherwise false.
		 */
		[[nodiscard]] bool isValid() const;
		
		/**
		 * @brief Return the used vertex buffer binding of the geometry data.
		 *
		 * @return Vertex buffer binding
		 */
		[[nodiscard]] const VertexBufferBinding &getVertexBufferBinding() const;
		
		/**
		 * @brief Return the stride of vertex elements of the geometry data.
		 *
		 * @return Vertex stride
		 */
		[[nodiscard]] uint32_t getVertexStride() const;
		
		/**
		 * @brief Return the maximal index from vertex elements of the geometry data.
		 *
		 * @return Maximal vertex index
		 */
		[[nodiscard]] size_t getMaxVertexIndex() const;
		
		/**
		 * @brief Return the geometry vertex type of the geometry data.
		 *
		 * @return Geometry vertex type
		 */
		[[nodiscard]] GeometryVertexType getGeometryVertexType() const;
		
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
