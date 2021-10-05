#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Features.hpp
 * @brief Class to manage feature requests.
 */

#include <functional>
#include <vector>
#include <initializer_list>

#include "FeatureManager.hpp"

namespace vkcv {
	
	/**
	 * Abstract function type to request a feature via the feature manager.
	 */
	typedef std::function<bool(FeatureManager&)> Feature;
	
	/**
	 * Class to manage a list of feature requests at once.
	 */
	class Features {
	private:
		/**
		 * List of feature requests.
		 */
		std::vector<Feature> m_features;
		
	public:
		/**
		 * @brief Constructor of a features instance.
		 */
		Features() = default;
		
		/**
		 * @brief Constructor of a features instance with a given list of extension identifier strings.
		 * @param list List of extension identifier strings
		 */
		Features(const std::initializer_list<std::string>& list);
		
		/**
		 * @brief Copy-constructor of a features instance.
		 * @param other Other features instance
		 */
		Features(const Features& other) = default;
		
		/**
		 * @brief Move-constructor of a features instance.
		 * @param other Other features instance
		 */
		Features(Features&& other) = default;
		
		/**
		 * @brief Destructor of a features instance.
		 */
		~Features() = default;
		
		/**
		 * @brief Copy-operator of a features instance.
		 * @param other Other features instance
		 * @return Reference to the features instance itself
		 */
		Features& operator=(const Features& other) = default;
		
		/**
		 * @brief Move-operator of a features instance.
		 * @param other Other features instance
		 * @return Reference to the features instance itself
		 */
		Features& operator=(Features&& other) = default;
		
		/**
		 * @brief Request a specific extension as required.
		 * @param extension Extension identifier string
		 */
		void requireExtension(const std::string& extension);
		
		/**
		 * @brief Request a specific extension and some of its features as required ( only core Vulkan 1.0 ).
		 * @param extension Extension identifier string
		 * @param featureFunction
		 */
		void requireExtensionFeature(const std::string& extension,
									 const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		/**
		 * @brief Request a specific extension and some of its features as required.
		 * @tparam T Template parameter to use specific base structure types
		 * @param extension Extension identifier string
		 * @param featureFunction Function or lambda to request specific features
		 */
		template<typename T>
		void requireExtensionFeature(const std::string& extension, const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([extension, featureFunction](FeatureManager& featureManager) {
				if (featureManager.useExtension(extension, true)) {
					return featureManager.template useFeatures<T>(featureFunction, true);
				} else {
					return false;
				}
			});
		}
		
		/**
		 * @brief Request a specific set of features as required ( only core Vulkan 1.0 ).
		 * @param featureFunction Function or lambda to request specific features
		 */
		void requireFeature(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		/**
		 * @brief Request a specific set of features as required.
		 * @tparam T Template parameter to use specific base structure types
		 * @param featureFunction Function or lambda to request specific features
		 */
		template<typename T>
		void requireFeature(const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([featureFunction](FeatureManager& featureManager) {
				return featureManager.template useFeatures<T>(featureFunction, true);
			});
		}
		
		/**
		 * @brief Request a specific extension as optional.
		 * @param extension Extension identifier string
		 */
		void tryExtension(const std::string& extension);
		
		/**
		 * @brief Request a specific extension and some of its features as optional ( only core Vulkan 1.0 ).
		 * @param extension Extension identifier string
		 * @param featureFunction Function or lambda to request specific features
		 */
		void tryExtensionFeature(const std::string& extension,
								 const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		/**
		 * @brief Request a specific extension and some of its features as optional.
		 * @tparam T Template parameter to use specific base structure types
		 * @param extension Extension identifier string
		 * @param featureFunction Function or lambda to request specific features
		 */
		template<typename T>
		void tryExtensionFeature(const std::string& extension, const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([extension, featureFunction](FeatureManager& featureManager) {
				if (featureManager.useExtension(extension, false)) {
					return featureManager.template useFeatures<T>(featureFunction, false);
				} else {
					return false;
				}
			});
		}
		
		/**
		 * @brief Request a specific set of features as optional ( only core Vulkan 1.0 ).
		 * @param featureFunction Function or lambda to request specific features
		 */
		void tryFeature(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		/**
		 * @brief Request a specific set of features as optional.
		 * @tparam T Template parameter to use specific base structure types
		 * @param featureFunction Function or lambda to request specific features
		 */
		template<typename T>
		void tryFeature(const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([featureFunction](FeatureManager& featureManager) {
				return featureManager.template useFeatures<T>(featureFunction, false);
			});
		}
		
		/**
		 * @brief Return list of feature requests.
		 * @return List of feature requests
		 */
		[[nodiscard]]
		const std::vector<Feature>& getList() const;
		
	};
	
}
