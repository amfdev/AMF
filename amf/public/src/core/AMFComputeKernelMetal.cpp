#include "AMFComputeKernelMetal.h"

AMFComputeKernelMetal::AMFComputeKernelMetal(AMF_KERNEL_ID kernelID, MetalComputeKernelWrapper * kernel)
  : m_kernelID(kernelID), m_kernel(kernel)
{

}

AMFComputeKernelMetal::~AMFComputeKernelMetal()
{
    delete m_kernel;
}

void * AMFComputeKernelMetal::GetNative()
{
    return NULL;
}

const wchar_t * AMFComputeKernelMetal::GetIDName()
{
    return L"AMFComputeKernelMetal";
}

AMF_RESULT    AMFComputeKernelMetal::SetArgPlaneNative(amf_size index, void* pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgBufferNative(amf_size index, void* pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgPlane(amf_size index, AMFPlane* pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgBuffer(amf_size index, AMFBuffer* pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return m_kernel->SetArgBuffer(pBuffer->GetNative(), index);
}

AMF_RESULT    AMFComputeKernelMetal::SetArgInt32(amf_size index, amf_int32 data)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgInt64(amf_size index, amf_int64 data)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgFloat(amf_size index, amf_float data)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::SetArgBlob(amf_size index, amf_size dataSize, const void* pData)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT    AMFComputeKernelMetal::GetCompileWorkgroupSize(amf_size workgroupSize[3])
{
    return m_kernel->GetCompileWorkgroupSize(workgroupSize);
}

AMF_RESULT    AMFComputeKernelMetal::Enqueue(amf_size dimension, amf_size globalOffset[3], amf_size globalSize[3], amf_size localSize[3])
{
    return m_kernel->Enqueue(globalSize, localSize);
}