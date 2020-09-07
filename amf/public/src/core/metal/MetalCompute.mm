#include "MetalCompute.h"

MetalCompute::MetalCompute(id<MTLDevice> device, id<MTLCommandQueue> commandQueue)
 :  m_device(device),
    m_commandQueue(commandQueue)
{
    m_commandBuffer = [m_commandQueue commandBuffer];
    assert(m_commandBuffer != nil);
    m_kernelBuffers = [NSMutableArray array];
    //m_library = [m_device newDefaultLibrary];
}

MetalCompute::~MetalCompute()
{
    [m_kernelBuffers removeAllObjects];
}

AMF_RESULT MetalCompute::GetKernel(NSString * source, NSString * name, MetalComputeKernel ** kernel)
{
    NSError* error = nil;
    MTLCompileOptions * options = [MTLCompileOptions new];

    if (@available(macOS 10.15, iOS 13.0, *))
        options.languageVersion = MTLLanguageVersion::MTLLanguageVersion2_2;

    m_library = [m_device newLibraryWithSource: source options:options error:&error];
    if (m_library == nil)
    {
        NSLog(@"Failed to createLibrary from source: %@ %@", error, [error userInfo]);

        return AMF_FAIL;
    }
    id<MTLFunction> processFunction = [m_library newFunctionWithName:name];
    if (processFunction == nil)
    {
        NSLog(@"Failed to find the process function.");
        return AMF_FAIL;
    }

    id<MTLComputePipelineState> processFunctionPSO = [m_device newComputePipelineStateWithFunction: processFunction error:&error];
    if (processFunctionPSO == nil)
    {
        //  If the Metal API validation is enabled, you can find out more information about what
        //  went wrong.  (Metal API validation is enabled by default when a debug build is run
        //  from Xcode)
        NSLog(@"Failed to created pipeline state object, error %@.", error);
        return AMF_FAIL;
    }

    id<MTLCommandBuffer> buffer = [m_commandQueue commandBuffer];
    [m_kernelBuffers addObject: buffer];
    (*kernel) = new MetalComputeKernel(buffer, processFunction, processFunctionPSO);
    return AMF_OK;
}

AMF_RESULT MetalCompute::FlushQueue()
{
    //[m_commandBuffer commit];
    for (id<MTLCommandBuffer> buffer in m_kernelBuffers)
    {
        [buffer commit];
    }
    return AMF_OK;
}

AMF_RESULT MetalCompute::FinishQueue()
{
    //[m_commandBuffer waitUntilCompleted];
    for (id<MTLCommandBuffer> buffer in m_kernelBuffers)
    {
        [buffer waitUntilCompleted];
    }
    return AMF_OK;
}


