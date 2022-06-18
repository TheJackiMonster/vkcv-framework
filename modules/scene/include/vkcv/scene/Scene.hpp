#pragma once

#include <filesystem>
#include <mutex>

#include <vkcv/Core.hpp>
#include <vkcv/Event.hpp>
#include <vkcv/camera/Camera.hpp>
#include <vkcv/material/Material.hpp>

#include "Node.hpp"

namespace vkcv::scene {

    /**
     * @defgroup vkcv_scene Scene Module
     * A module to manage basic scene rendering with CPU-side frustum culling.
     * @{
     */

    /**
     * A class to represent a scene graph.
     */
	class Scene {
		friend class MeshPart;
		
	private:
        /**
         * A nested structure to manage material storage.
         */
		struct Material {
            /**
             * The amount of active usages of the material.
             */
			size_t m_usages;

            /**
             * The actual material data.
             */
			material::Material m_data;
		};

        /**
         * A pointer to the current Core instance.
         */
		Core* m_core;

        /**
         * A list of currently used materials.
         */
		std::vector<Material> m_materials;

        /**
         * A list of nodes in the first level of the scene graph.
         */
		std::vector<Node> m_nodes;
		
		/**
		 * A handle to the descriptor set layout used by meshes in this scene.
		 */
		DescriptorSetLayoutHandle m_descriptorSetLayout;

        /**
         * Constructor of a scene instance with a given Core instance.
         * @param[in,out] core Pointer to valid Core instance
         */
		explicit Scene(Core* core);

        /**
         * Add a new node to the first level of the scene graph with
         * this scene as parent and return its index.
         * @return Index of the new node
         */
		size_t addNode();

        /**
         * Get a reference to the first-level node with a given index.
         * @param[in] index Valid index of a first-level node
         * @return Matching first-level node
         */
		Node& getNode(size_t index);

        /**
        * Get a const reference to the first-level node with a given index.
        * @param[in] index Valid index of a first-level node
        * @return Matching first-level node
        */
        [[nodiscard]]
		const Node& getNode(size_t index) const;

        /**
         * Increase the amount of usages for a certain material via its index.
         * @param[in] index Index of a material
         */
		void increaseMaterialUsage(size_t index);

        /**
         * Decrease the amount of usages for a certain material via its index.
         * @param[in] index Index of a material
         */
		void decreaseMaterialUsage(size_t index);

        /**
         * Load a material from a scene preloaded with the asset loader via
         * its index.
         * @param[in] index Valid index of a material
         * @param[in] scene Scene structure from asset loader
         * @param[in] material Material structure from asset loader
         */
		void loadMaterial(size_t index, const asset::Scene& scene,
						  const asset::Material& material);
		
	public:
        /**
         * Destructor of a scene instance.
         */
		~Scene();

        /**
         * Copy-constructor of a scene instance.
         * @param[in] other Other scene instance
         */
		Scene(const Scene& other);

        /**
         * Move-constructor of a scene instance.
         * @param[in,out] other Other scene instance
         */
        Scene(Scene&& other) noexcept;

        /**
         * Copy-operator of a scene instance.
         * @param[in] other Other scene instance
         * @return Reference to this scene
         */
		Scene& operator=(const Scene& other);

        /**
         * Move-operator of a scene instance.
         * @param[in,out] other Other scene instance
         * @return Reference to this scene
         */
        Scene& operator=(Scene&& other) noexcept;

        /**
         * Return the amount of materials managed by this scene.
         * @return Amount of materials
         */
        [[nodiscard]]
		size_t getMaterialCount() const;

        /**
         * Get the material data by its certain index.
         * The material can still be invalid if it was not loaded properly.
         * @param[in] index Valid index of material
         * @return Material
         */
		[[nodiscard]]
		const material::Material& getMaterial(size_t index) const;
		
		/**
         * Returns the descriptor set layout handle used by the meshes in the scene.
         * @return Descriptor set layout handle
         */
		[[nodiscard]]
		const DescriptorSetLayoutHandle& getDescriptorSetLayout() const;

        /**
         * Record drawcalls of all meshes of this scene with CPU-side frustum culling.
         * @param cmdStream Command stream handle
         * @param camera Scene viewing camera
         * @param pass Render pass handle
         * @param pipeline Graphics pipeline handle
         * @param pushConstantsSizePerDrawcall Size of push constants per drawcall
         * @param record Drawcall recording event function
         * @param renderTargets Actual render targets
         * @param windowHandle Window handle to use
         */
		void recordDrawcalls(CommandStreamHandle       		  &cmdStream,
							 const camera::Camera			  &camera,
							 const PassHandle                 &pass,
							 const GraphicsPipelineHandle     &pipeline,
							 size_t							  pushConstantsSizePerDrawcall,
							 const RecordMeshDrawcallFunction &record,
							 const std::vector<ImageHandle>   &renderTargets,
							 const WindowHandle               &windowHandle);

        /**
         * Instantiation function to create a new scene instance.
         * @param core Current Core instance
         * @return New scene instance
         */
		static Scene create(Core& core);

        /**
         * Load function to create a new scene instance with materials and meshes
         * loaded from a file using the asset loader.
         * @param core Current Core instance
         * @param path Path of a valid file to load via asset loader
         * @return New scene instance
         */
		static Scene load(Core& core, const std::filesystem::path &path);
		
	};

    /** @} */
	
}