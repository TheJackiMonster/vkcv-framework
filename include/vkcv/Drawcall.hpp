#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Drawcall.hpp
 * @brief Classes to define different drawcalls.
 */

#include "Container.hpp"
#include "DescriptorSetUsage.hpp"
#include "DispatchSize.hpp"
#include "Handles.hpp"
#include "VertexData.hpp"

namespace vkcv {

	/**
	 * @brief Base class to store details for a general drawcall.
	 */
	class Drawcall {
	private:
		Vector<DescriptorSetUsage> m_usages;

	public:
		Drawcall() = default;

		Drawcall(const Drawcall &other) = default;
		Drawcall(Drawcall &&other) noexcept = default;

		~Drawcall() = default;

		Drawcall &operator=(const Drawcall &other) = default;
		Drawcall &operator=(Drawcall &&other) noexcept = default;

		[[nodiscard]] const Vector<DescriptorSetUsage> &getDescriptorSetUsages() const;

		void useDescriptorSet(uint32_t location, const DescriptorSetHandle &descriptorSet,
							  const Vector<uint32_t> &dynamicOffsets = {});
	};

	/**
	 * @brief Class to store details for an instance drawcall.
	 */
	class InstanceDrawcall : public Drawcall {
	private:
		VertexData m_vertexData;
		uint32_t m_instanceCount;

	public:
		explicit InstanceDrawcall(const VertexData &vertexData, uint32_t instanceCount = 1);

		[[nodiscard]] const VertexData &getVertexData() const;

		[[nodiscard]] uint32_t getInstanceCount() const;
	};

	/**
	 * @brief Class to store details for an indirect drawcall.
	 */
	class IndirectDrawcall : public Drawcall {
	private:
		BufferHandle m_indirectDrawBuffer;
		VertexData m_vertexData;
		uint32_t m_drawCount;

	public:
		explicit IndirectDrawcall(const BufferHandle &indirectDrawBuffer,
								  const VertexData &vertexData, uint32_t drawCount = 1);

		[[nodiscard]] BufferHandle getIndirectDrawBuffer() const;

		[[nodiscard]] const VertexData &getVertexData() const;

		[[nodiscard]] uint32_t getDrawCount() const;
	};

	/**
	 * @brief Class to store details for a task drawcall.
	 */
	class TaskDrawcall : public Drawcall {
	private:
		DispatchSize m_taskSize;

	public:
		explicit TaskDrawcall(const DispatchSize& taskSize = DispatchSize(1));

		[[nodiscard]] DispatchSize getTaskSize() const;
	};

} // namespace vkcv
