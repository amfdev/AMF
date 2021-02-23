#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "MetalCompute.h"
#include "../../../include/core/Result.h"

class MetalDevice
{
public:

    MetalDevice(id<MTLDevice> device);
    MetalDevice();

    id<MTLBuffer> AllocateBuffer(size_t size);
    AMF_RESULT ReleaseBuffer(id<MTLBuffer>  buffer);

    AMF_RESULT CopyBuffer(id<MTLBuffer> pDestHandle, size_t dstOffset, id<MTLBuffer> pSourceHandle, size_t srcOffset, size_t size);
    AMF_RESULT CopyBufferToHost(void *pDest, id<MTLBuffer> pSourceHandle, size_t srcOffset, size_t size, bool blocking);
    AMF_RESULT CopyBufferFromHost(id<MTLBuffer> pDestHandle, size_t dstOffset, const void *pSource, size_t size, bool blocking);

    AMF_RESULT CreateCompute(MetalCompute ** compute);
    AMF_RESULT CreateSubBuffer(id<MTLBuffer> pSourceHandle, void ** subBuffer, amf_size offset, amf_size size);
    AMF_RESULT FillBuffer(id<MTLBuffer> pDestHandle, amf_size dstOffset, amf_size dstSize, const void *pSourcePattern, amf_size patternSize);

    id<MTLDevice> GetNativeDevice();
    id<MTLCommandQueue> GetNativeCommandQueue();
private:
    NSUInteger AlignedValue(NSUInteger value, bool toLower = false);
private:
    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_defaultCommandQueue;
    NSUInteger m_pageSize;
};