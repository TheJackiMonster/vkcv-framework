#pragma once

#include <functional>
#include <vector>

#include "FeatureManager.hpp"

namespace vkcv {
	
	typedef std::function<bool(FeatureManager&)> Feature;
	
	class Features {
	private:
		std::vector<Feature> m_features;
		
	public:
		Features() = default;
		Features(const Features& other) = default;
		Features(Features&& other) = default;
		~Features() = default;
		
		Features& operator=(const Features& other) = default;
		Features& operator=(Features&& other) = default;
		
		void requireExtension(const std::string& extension);
		
		void requireExtensionFeature(const std::string& extension,
									 const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		template<typename T>
		void requireExtensionFeature(const std::string& extension, const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([extension, &featureFunction](FeatureManager& featureManager) {
				if (featureManager.useExtension(extension, true)) {
					return featureManager.template useFeatures<T>(featureFunction, true);
				} else {
					return false;
				}
			});
		}
		
		void requireFeature(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		template<typename T>
		void requireFeature(const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([&featureFunction](FeatureManager& featureManager) {
				return featureManager.template useFeatures<T>(featureFunction, true);
			});
		}
		
		void tryExtension(const std::string& extension);
		
		void tryExtensionFeature(const std::string& extension,
								 const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		template<typename T>
		void tryExtensionFeature(const std::string& extension, const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([extension, &featureFunction](FeatureManager& featureManager) {
				if (featureManager.useExtension(extension, false)) {
					return featureManager.template useFeatures<T>(featureFunction, false);
				} else {
					return false;
				}
			});
		}
		
		void tryFeature(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction);
		
		template<typename T>
		void tryFeature(const std::function<void(T&)>& featureFunction) {
			m_features.emplace_back([&featureFunction](FeatureManager& featureManager) {
				return featureManager.template useFeatures<T>(featureFunction, false);
			});
		}
		
		[[nodiscard]]
		const std::vector<Feature>& getList() const;
		
	};
	
}
