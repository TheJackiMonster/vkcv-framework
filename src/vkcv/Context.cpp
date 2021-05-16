#include <iostream>
#include "vkcv/Context.hpp"

namespace vkcv
{
    Context::Context(Context &&other) noexcept:
            m_Instance(other.m_Instance),
            m_PhysicalDevice(other.m_PhysicalDevice),
            m_Device(other.m_Device)
    {
        other.m_Instance        = nullptr;
        other.m_PhysicalDevice  = nullptr;
        other.m_Device          = nullptr;
    }

    Context & Context::operator=(Context &&other) noexcept
    {
        m_Instance          = other.m_Instance;
        m_PhysicalDevice    = other.m_PhysicalDevice;
        m_Device            = other.m_Device;

        other.m_Instance        = nullptr;
        other.m_PhysicalDevice  = nullptr;
        other.m_Device          = nullptr;

        return *this;
    }

    Context::Context(vk::Instance instance,
                     vk::PhysicalDevice physicalDevice,
                     vk::Device device) noexcept :
    m_Instance{instance},
    m_PhysicalDevice{physicalDevice},
    m_Device{device}
    {}

    Context::~Context() noexcept
    {
        std::cout<< " Context " << std::endl;
        m_Device.destroy();
        m_Instance.destroy();
    }

    const vk::Instance &Context::getInstance() const
    {
        return m_Instance;
    }

    const vk::PhysicalDevice &Context::getPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }

    const vk::Device &Context::getDevice() const
    {
        return m_Device;
    }
}
