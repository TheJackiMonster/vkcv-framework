#include "RTX.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule(Core* core,
						 ASManager* asManager,
						 const vkcv::VertexData &vertexData,
						 std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles){
        m_core = core;
        m_asManager = asManager;
        // build acceleration structures BLAS then TLAS --> see ASManager
        m_asManager->buildBLAS(vertexData);
        m_asManager->buildTLAS();
        RTXDescriptors(descriptorSetHandles);
    }


    RTXModule::~RTXModule()
    {}


    void RTXModule::RTXDescriptors(std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles)
    {
        //TLAS-Descriptor-Write
		{
			vkcv::DescriptorWrites writes;
			writes.writeAcceleration(1, { m_asManager->getTLAS().vulkanHandle });
			m_core->writeDescriptorSet(descriptorSetHandles[0], writes);
		}

        //INDEX & VERTEX BUFFER
        BottomLevelAccelerationStructure blas = m_asManager->getBLAS(0);//HARD CODED

        //VERTEX BUFFER
        vk::DescriptorBufferInfo vertexInfo = {};
        vertexInfo.setBuffer(blas.vertexBuffer.vulkanHandle);
        vertexInfo.setOffset(0);
        vertexInfo.setRange(blas.vertexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet vertexWrite;
        vertexWrite.setDstSet(m_core->getVulkanDescriptorSet(descriptorSetHandles[0]));
        vertexWrite.setDstBinding(2);
        vertexWrite.setDescriptorCount(1);
        vertexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        vertexWrite.setPBufferInfo(&vertexInfo);
        m_core->getContext().getDevice().updateDescriptorSets(vertexWrite, nullptr);

        //INDEXBUFFER
        vk::DescriptorBufferInfo indexInfo = {};
        indexInfo.setBuffer(blas.indexBuffer.vulkanHandle);
        indexInfo.setOffset(0);
        indexInfo.setRange(blas.indexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet indexWrite;
        indexWrite.setDstSet(m_core->getVulkanDescriptorSet(descriptorSetHandles[0]));
        indexWrite.setDstBinding(3);
        indexWrite.setDescriptorCount(1);
        indexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        indexWrite.setPBufferInfo(&indexInfo);
        m_core->getContext().getDevice().updateDescriptorSets(indexWrite, nullptr);
    }

}