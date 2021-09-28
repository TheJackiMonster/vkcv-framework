#pragma once
#include <vector>
#include <vkcv/Core.hpp>

namespace vkcv::rtx{

class RTXExtensions {
private:
    std::vector<const char*> m_instanceExtensions;  // the instance extensions needed for using RTX
    std::vector<const char*> m_deviceExtensions;    // the device extensions needed for using RTX
    vkcv::Features m_features;                      // the features needed to be enabled for using RTX
public:

    RTXExtensions();
    ~RTXExtensions()=default;
    /**
     * @brief Returns the raytracing instance extensions.
     * @return The raytracing instance extensions.
     */
    std::vector<const char*> getInstanceExtensions();

    /**
     * @brief Returns the raytracing device extensions.
     * @return The raytracing device extensions.
     */
    std::vector<const char*> getDeviceExtensions();

    /**
     * @brief Returns the raytracing features.
     * @return The raytracing features.
     */
    vkcv::Features getFeatures();
                    

};
}