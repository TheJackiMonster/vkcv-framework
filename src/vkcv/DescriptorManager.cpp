#include "DescriptorManager.hpp"


namespace vkcv
{
    ResourcesHandle vkcv::DescriptorManager::createResourceDescription(const std::vector<SetDescription> &setDescriptions)
    {
        // TODO: create all vk::DescriptorSets and allocate them from the pool
        // put them into a ResourceDescription struct
        // push that struct into m_Resources;
        // return the index into that object as ResourcesHandle;
        return ResourcesHandle{0};
    }
}