#include "MetalComputeKernel.h"

#include "public/common/TraceAdapter.h"

#include <thread>
#include <iostream>

#define AMF_FACILITY L"MetalComputeKernel"

MetalComputeKernel::MetalComputeKernel(
    id<MTLCommandQueue>         queue,
    id<MTLFunction>             processFunction,
    id<MTLComputePipelineState> processFunctionPSO
    ):
    mCommandQueue(queue),
    m_processFunction(processFunction),
    m_processFunctionPSO(processFunctionPSO)
{
}

MetalComputeKernel::~MetalComputeKernel()
{
}

AMF_RESULT MetalComputeKernel::SetArgBuffer(id<MTLBuffer> buffer, int index)
{
    Bindind bind;
    bind.type = Bindind::Buffer;
    bind.buffer = buffer;
    bind.index = index;
    m_bindings.push_back(bind);

    return AMF_OK;
}

AMF_RESULT MetalComputeKernel::SetArgInt32(int32_t value, int index)
{
    Bindind bind;
    bind.type = Bindind::Int32;
    bind.value32 = value;
    bind.index = index;
    m_bindings.push_back(bind);

    return AMF_OK;
}

AMF_RESULT MetalComputeKernel::SetArgInt64(int64_t value, int index)
{
    Bindind bind;
    bind.type = Bindind::Int64;
    bind.value64 = value;
    bind.index = index;
    m_bindings.push_back(bind);

    return AMF_OK;
}

AMF_RESULT MetalComputeKernel::SetArgFloat(float value, int index)
{
    Bindind bind;
    bind.type = Bindind::Float;
    bind.valueFloat = value;
    bind.index = index;
    m_bindings.push_back(bind);

    return AMF_OK;
}

MTLSize MetalComputeKernel::GetCompileWorkgroupSize(MTLSize maxSize)
{
    NSUInteger w = m_processFunctionPSO.threadExecutionWidth;
    if (w > maxSize.width)
        w = maxSize.width;
    NSUInteger h = m_processFunctionPSO.maxTotalThreadsPerThreadgroup / w;
    if (h > maxSize.height)
        h = maxSize.height;

    //int d = m_processFunctionPSO.maxTotalThreadsPerThreadgroup / (w * h);

    return MTLSizeMake(w, h, 1);
}

AMF_RESULT MetalComputeKernel::Enqueue(MTLSize workgroupSize, MTLSize sizeInWorkgroup)
{
    m_workgroupSize = workgroupSize;
    m_sizeInWorkgroup = sizeInWorkgroup;

    @autoreleasepool
    {
        id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoderWithDispatchType:MTLDispatchTypeConcurrent];

        [commandBuffer addScheduledHandler:
            ^(id<MTLCommandBuffer> commandBuffer)
            {
                //NSLog(@"scheduled");
            }
            ];
        [commandBuffer addCompletedHandler:
            ^(id<MTLCommandBuffer> commandBuffer)
            {
                //NSLog(@"completed");
            }
            ];

        [encoder setComputePipelineState:m_processFunctionPSO];

        for(MetalComputeKernel::Bindind & bind : m_bindings)
        {
            switch(bind.type)
            {
            case Bindind::Buffer:
                [encoder setBuffer:bind.buffer offset:0 atIndex:bind.index];
                break;
            case MetalComputeKernel::Bindind::Int32:
                [encoder setBytes:&bind.value32 length:sizeof(int32_t) atIndex:bind.index];
                break;
            case MetalComputeKernel::Bindind::Int64:
                [encoder setBytes:&bind.value64 length:sizeof(int64_t) atIndex:bind.index];
                break;
            case MetalComputeKernel::Bindind::Float:
                [encoder setBytes:&bind.valueFloat length:sizeof(float) atIndex:bind.index];
                break;
            default:
                break;
            }
        }

        [encoder
            dispatchThreads:m_workgroupSize
            threadsPerThreadgroup:m_sizeInWorkgroup];
        [encoder endEncoding];

        [commandBuffer enqueue];
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }

    return Reset();
}

AMF_RESULT MetalComputeKernel::FlushQueue()
{
    return AMF_OK;
}

AMF_RESULT MetalComputeKernel::FinishQueue()
{
    /*
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoderWithDispatchType:MTLDispatchTypeConcurrent];
        [encoder setComputePipelineState:m_processFunctionPSO];

        for(MetalComputeKernel::Bindind & bind : m_bindings)
        {
            switch (bind.type) {
                case Bindind::Buffer:
                    [encoder setBuffer:bind.buffer offset:0 atIndex:bind.index];
                    break;
                case MetalComputeKernel::Bindind::Int32:
                    [encoder setBytes:&bind.value32 length:sizeof(int32_t) atIndex:bind.index];
                    break;
                case MetalComputeKernel::Bindind::Int64:
                    [encoder setBytes:&bind.value64 length:sizeof(int64_t) atIndex:bind.index];
                    break;
                case MetalComputeKernel::Bindind::Float:
                    [encoder setBytes:&bind.valueFloat length:sizeof(float) atIndex:bind.index];
                    break;
                default:
                    break;
            }
        }
        [encoder dispatchThreads:m_workgroupSize
                threadsPerThreadgroup:m_sizeInWorkgroup];
        [encoder endEncoding];
        //[commandBuffer enqueue];
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }
    return Reset();
    */

    return AMF_OK;
}

AMF_RESULT MetalComputeKernel::Reset()
{
    m_bindings.clear();
    return AMF_OK;
}
