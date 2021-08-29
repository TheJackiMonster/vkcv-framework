
#include "vkcv/Features.hpp"

namespace vkcv {
	
	void Features::requireExtension(const std::string& extension) {
		m_features.emplace_back([extension](FeatureManager& featureManager) {
			return featureManager.useExtension(extension, true);
		});
	}
	
	void Features::requireExtensionFeature(const std::string &extension,
										   const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction) {
		m_features.emplace_back([extension, &featureFunction](FeatureManager& featureManager) {
			if (featureManager.useExtension(extension, true)) {
				return featureManager.useFeatures(featureFunction, true);
			} else {
				return false;
			}
		});
	}
	
	void Features::requireFeature(const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction) {
		m_features.emplace_back([&featureFunction](FeatureManager& featureManager) {
			return featureManager.useFeatures(featureFunction, true);
		});
	}
	
	void Features::tryExtension(const std::string& extension) {
		m_features.emplace_back([extension](FeatureManager& featureManager) {
			return featureManager.useExtension(extension, false);
		});
	}
	
	void Features::tryExtensionFeature(const std::string &extension,
									   const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction) {
		m_features.emplace_back([extension, &featureFunction](FeatureManager& featureManager) {
			if (featureManager.useExtension(extension, false)) {
				return featureManager.useFeatures(featureFunction, false);
			} else {
				return false;
			}
		});
	}
	
	void Features::tryFeature(const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction) {
		m_features.emplace_back([&featureFunction](FeatureManager& featureManager) {
			return featureManager.useFeatures(featureFunction, false);
		});
	}
	
	const std::vector<Feature>& Features::getList() const {
		return m_features;
	}

}
