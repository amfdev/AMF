#include "ComputeOCL.h"
#include "ProgramsImpl.h"
#include "DeviceOCLImpl.h"

static uint32_t amf_to_cl_format(enum AMF_ARGUMENT_ACCESS_TYPE format)
{
    if (format == AMF_ARGUMENT_ACCESS_READWRITE)
        return CL_MEM_READ_WRITE;
    if (format == AMF_ARGUMENT_ACCESS_WRITE)
        return CL_MEM_WRITE_ONLY;
    if (format == AMF_ARGUMENT_ACCESS_READ)
        return CL_MEM_READ_ONLY;

    return CL_MEM_READ_ONLY;
}

AMF_RESULT AMFComputeFactoryOCL::Init()
{
    //cl_platform_id platformID;
    amf_vector<cl_platform_id> platforms;

    cl_uint numPlatforms = 0;
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    AMF_RETURN_IF_CL_FAILED(status, L"clGetPlatformIDs() failed");

    if(numPlatforms > 0)
    {
        platforms.resize(numPlatforms);

        status = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
        AMF_RETURN_IF_CL_FAILED(status, L"clGetPlatformIDs() failed");

        for(unsigned i = 0; i < numPlatforms; ++i)
        {
            char pbuf[1000] = {0};
            status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(pbuf), pbuf, NULL);
            AMF_RETURN_IF_CL_FAILED(status, L"clGetPlatformInfo() failed");
            m_pContext->SetProperty(
                AMF_DRIVER_VERSION_NAME,
                AMFVariant(pbuf)
                );
            /*if(!strcmp(pbuf, "Advanced Micro Devices, Inc."))
            {
                platformID = platforms[i];
                break;
            }*/
        }
    }

    //try to load device type property, if it is set
    amf::AMFVariant contextType;
    auto result = m_pContext->GetProperty(AMF_CONTEXT_DEVICE_TYPE, &contextType);

    bool contextTypeSet(false);
    AMF_CONTEXT_DEVICETYPE_ENUM deviceType(AMF_CONTEXT_DEVICE_TYPE_GPU);

    if(AMFSucceeded(result))
    {
        deviceType = AMF_CONTEXT_DEVICETYPE_ENUM(int(contextType));
        contextTypeSet = true;
    }

    amf_vector<cl_device_id> deviceIDs;

    //if(platformID)
    for(size_t platformIndex(0); platformIndex < platforms.size(); ++platformIndex)
    {
        auto platformID = platforms[platformIndex];

        cl_uint numDevices;
        std::vector<cl_context_properties> cps;

        //does not work on mac with OpenCL 1.2
        //cps.push_back(CL_CONTEXT_INTEROP_USER_SYNC);
        //cps.push_back(CL_TRUE);

        cps.push_back(CL_CONTEXT_PLATFORM);
        cps.push_back((cl_context_properties)platformID);
        cps.push_back(0);

        //status = clGetDeviceIDs((cl_platform_id)platformID, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
        //improvement to control what devices types to retrieve:
        status = clGetDeviceIDs(
            (cl_platform_id)platformID,
            contextTypeSet && (AMF_CONTEXT_DEVICE_TYPE_CPU == deviceType)
                ? CL_DEVICE_TYPE_CPU
                : CL_DEVICE_TYPE_GPU,
            0,
            nullptr,
            &numDevices
            );

        if(numDevices)
        {
            deviceIDs.resize(numDevices);

            status = clGetDeviceIDs(
                (cl_platform_id)platformID,
                contextTypeSet && (AMF_CONTEXT_DEVICE_TYPE_CPU == deviceType)
                    ? CL_DEVICE_TYPE_CPU
                    : CL_DEVICE_TYPE_GPU,
                numDevices,
                (cl_device_id*)&deviceIDs[0],
                nullptr
                );
        }

        for (int i = 0; i < numDevices; ++i)
        {
            cl_context context = clCreateContext(&cps[0], 1, &deviceIDs[i], NULL, NULL, &status);
            m_devices.push_back(new AMFDeviceOCLImpl(platformID, deviceIDs[i], m_pContext, context));
        }
    }
	return AMF_OK;
}

amf_int32 AMFComputeFactoryOCL::GetDeviceCount()
{
    return m_devices.size();
}

AMF_RESULT AMFComputeFactoryOCL::GetDeviceAt(amf_int32 index, AMFComputeDevice **ppDevice)
{
    //TODO: check out of range
    AMFDeviceOCLImpl* deviceImpl = dynamic_cast<AMFDeviceOCLImpl*>(m_devices.at(index).GetPtr());
    *ppDevice = deviceImpl->GetComputeDevice();
    (*ppDevice)->Acquire();
    return AMF_OK;
}

//****************** end AMFComputeDeviceOCL ******************


//****************** AMFComputeKernelOCL ******************
void *AMFComputeKernelOCL::GetNative()
{
    return m_kernel;
}

AMF_RESULT AMFComputeKernelOCL::Enqueue(
    amf_size dimension,
    amf_size globalOffset[3],
    amf_size globalSize[3],
    amf_size localSize[3]
    )
{
    AMF_RETURN_IF_FALSE(
        !globalOffset || (!globalOffset[0] && !globalOffset[1] && !globalOffset[2]),
        AMF_NOT_SUPPORTED,
        L"Error: offsets are not supported!"
        );

    int error = clEnqueueNDRangeKernel(
        m_command_queue,
        m_kernel,
        dimension,
        nullptr, //reference documentation say that globalOffset must be NULL
        &globalSize[0],
        &localSize[0],
        0,
        NULL,
        NULL
        );
    AMF_RETURN_IF_CL_FAILED(error, L"Error: Failed to execute kernel!");

    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::GetCompileWorkgroupSize(amf_size workgroupSizes[3])
{
    size_t returnedWorkgroupSizes[3] = {0};
    size_t returnedSize(0);
    cl_int returnedCode = clGetKernelWorkGroupInfo(
        m_kernel,
        m_deviceID,
        CL_KERNEL_WORK_GROUP_SIZE,
        3 * sizeof(size_t),
        returnedWorkgroupSizes,
        &returnedSize
        );
    AMF_RETURN_IF_CL_FAILED(returnedCode, L"Error: failed to retrieve kernel work group info!");

    workgroupSizes[0] = amf_size(returnedWorkgroupSizes[0]);
    workgroupSizes[1] = amf_size(returnedWorkgroupSizes[1]);
    workgroupSizes[2] = amf_size(returnedWorkgroupSizes[2]);

    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::SetArgBlob(amf_size index, amf_size dataSize, const void *pData)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeKernelOCL::SetArgFloat(amf_size index, amf_float data)
{
    int err = 0;
    err = clSetKernelArg(m_kernel, index, sizeof(amf_float), &data);
    if (err != 0)
    {
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::SetArgInt64(amf_size index, amf_int64 data)
{
    int err = 0;
    err = clSetKernelArg(m_kernel, index, sizeof(amf_int64), &data);
    if (err != CL_SUCCESS)
    {
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::SetArgInt32(amf_size index, amf_int32 data)
{
    int err = 0;
    err = clSetKernelArg(m_kernel, index, sizeof(amf_int32), &data);
    if (err != 0)
    {
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::SetArgBuffer(amf_size index, AMFBuffer *pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    int err = 0;
    AMF_RESULT res = AMF_FAIL;

    if (pBuffer->GetMemoryType() != AMF_MEMORY_OPENCL)
    {
        AMF_RETURN_IF_FAILED(
            pBuffer->Convert(AMF_MEMORY_OPENCL),
            L"Error: Failed to Convert arg buffer to OpenCl memory!"
            );
    }

    return SetArgBufferNative(index, pBuffer->GetNative(), eAccess);
}

AMF_RESULT AMFComputeKernelOCL::SetArgPlane(amf_size index, AMFPlane *pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeKernelOCL::SetArgBufferNative(amf_size index, void *pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    int err = 0;

    err = clSetKernelArg(m_kernel, index, sizeof(cl_mem), (cl_mem *)&pBuffer);
    if (err != 0)
    {
        printf("Error: Failed to setup arg buffer! Code = %d\n index = %d", err, index);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeKernelOCL::SetArgPlaneNative(amf_size index, void *pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess)
{
    return AMF_NOT_IMPLEMENTED;
}

const wchar_t *AMFComputeKernelOCL::GetIDName()
{
    return L"AMFComputeKernelOCL";
}

AMFComputeKernelOCL::AMFComputeKernelOCL(cl_program program, cl_kernel kernel, cl_command_queue command_queue, AMF_KERNEL_ID kernelID, cl_device_id deviceID, cl_context context)
    : m_program (program), m_kernel(kernel), m_kernelID(kernelID), m_command_queue(command_queue), m_deviceID(deviceID), m_context(context)
{


}

AMFComputeKernelOCL::~AMFComputeKernelOCL()
{
	clReleaseProgram(m_program);
	clReleaseKernel(m_kernel);
}
