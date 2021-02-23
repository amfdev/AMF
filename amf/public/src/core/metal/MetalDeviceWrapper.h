#pragma once

#include "../../../include/core/Result.h"
#include "MetalComputeWrapper.h"

class MetalDevice;

class MetalDeviceWrapper
{
public:
    MetalDeviceWrapper(void * native);
    ~MetalDeviceWrapper();

    void * AllocateBuffer(size_t size);
    AMF_RESULT ReleaseBuffer(void * native);

    AMF_RESULT CopyBuffer(void * pDestHandle, size_t dstOffset, void * pSourceHandle, size_t srcOffset, size_t size);
    AMF_RESULT CopyBufferToHost(void *pDest, void * pSourceHandle, size_t srcOffset, size_t size, bool blocking);
    AMF_RESULT CopyBufferFromHost(void * pDestHandle, size_t dstOffset, const void *pSource, size_t size, bool blocking);

    AMF_RESULT CreateCompute(MetalComputeWrapper ** compute);
    AMF_RESULT CreateSubBuffer(void* pHandle, void ** subBuffer, amf_size offset, amf_size size);
    AMF_RESULT FillBuffer(void *pDestHandle, amf_size dstOffset, amf_size dstSize, const void *pSourcePattern, amf_size patternSize);
    void * GetNativeDevice();

private:
    MetalDevice *mDevice = nullptr;
};