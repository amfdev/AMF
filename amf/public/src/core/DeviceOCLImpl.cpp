#include "DeviceOCLImpl.h"
#include <CL/cl.h>
#include "ComputeOCL.h"
#include "ProgramsImpl.h"

#include <iostream>
#include <fstream>

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

std::string GetCLDeviceNameByID(cl_device_id device)
{
    std::string name;

	size_t size(0);
    AMF_ASSERT(CL_SUCCESS == clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &size) && size);

    name.resize(size);
	AMF_ASSERT(CL_SUCCESS == clGetDeviceInfo(device, CL_DEVICE_NAME, size, &name.front(), 0));

    //eliminate issue with invalid size on Mac (size returned with additional null at the end)
    auto nameStrlen(strnlen(name.c_str(), name.size()));

    if(nameStrlen && nameStrlen < size)
    {
        name.resize(nameStrlen);
    }

    return name;
}

bool SaveProgramBinary(cl_program program, cl_device_id device, const wchar_t* name, const char* kernelName)
{
	cl_uint numDevices = 0;
	cl_int errNum = 0;

	errNum = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
		sizeof(cl_uint),
		&numDevices, NULL);
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error querying for number of devices." << std::endl;

		return false;
	}

	std::vector<cl_device_id> devices(numDevices);
	errNum = clGetProgramInfo(
        program,
        CL_PROGRAM_DEVICES,
		sizeof(cl_device_id) * numDevices,
		&devices.front(),
        NULL
        );
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error querying for devices." << std::endl;

        return false;
	}

	std::vector<size_t> programBinarySizes(numDevices);
	errNum = clGetProgramInfo(
        program,
        CL_PROGRAM_BINARY_SIZES,
		sizeof(size_t) * numDevices,
		&programBinarySizes.front(),
        NULL
        );
	if (errNum != CL_SUCCESS)
	{
        std::cerr << "Error querying for program binary sizes." << std::endl;

        return false;
	}

	std::vector<std::vector<amf_uint8>> programBinariesStorage(numDevices);
    std::vector<unsigned char*> programBinaries(numDevices);

	for (cl_uint i = 0; i < numDevices; i++)
	{
        programBinariesStorage[i].resize(programBinarySizes[i]);
		programBinaries[i] = &programBinariesStorage[i].front();
	}

	errNum = clGetProgramInfo(
        program,
        CL_PROGRAM_BINARIES,
		sizeof(unsigned char*) * numDevices,
		&programBinaries.front(),
        NULL
        );

	if (errNum != CL_SUCCESS)
	{
        std::cerr << "Error querying for program binaries." << std::endl;

		return false;
	}

	auto deviceName = GetCLDeviceNameByID(device);

	for (cl_uint i = 0; i < numDevices; i++)
	{
		if (devices[i] == device)
		{
			AMF_RETURN_IF_FALSE(
                AMF_OK != AMFKernelStorage::Instance()->SaveProgramBinary(
                    name,
                    kernelName,
                    deviceName,
                    ".cl.bin",
                    programBinaries[i],
                    programBinarySizes[i]
                    ),
                false
                );

			break;
		}
	}

    return true;
}

cl_program CreateProgramFromBinary(
    cl_context                      context,
    cl_device_id                    device,
    const std::vector<amf_uint8> &  programBinary
    )
{
	cl_int errNum = 0;
	cl_program program = nullptr;
	cl_int binaryStatus = 0;

    size_t      binariesSizes[]     = {programBinary.size()};
    auto binaries = &programBinary.front();

	program = clCreateProgramWithBinary(
        context,
		1,
		&device,
		binariesSizes,
		&binaries,
		&binaryStatus,
		&errNum
        );

    if (errNum != CL_SUCCESS)
	{
		std::cerr << "Error loading program binary." << std::endl;

		return NULL;
	}

	if (binaryStatus != CL_SUCCESS)
	{
		std::cerr << "Invalid binary for device." << std::endl;

		return NULL;
	}

	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    if (errNum != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);
		std::cerr << "Error in program: " << std::endl << buildLog << std::endl;

		clReleaseProgram(program);

        return NULL;
	}

	return program;
}


//AMFComputeDevice interface

AMFComputeDeviceOCLImpl::AMFComputeDeviceOCLImpl(AMFDeviceImpl* device, AMFContextImpl* pContext, cl_platform_id platform, cl_device_id deviceId, cl_context context)
    :
    m_platformID(platform),
    m_deviceID(deviceId),
    m_context(context),
    m_pContext(pContext),
    m_device(device)
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeDeviceOCLImpl", 0, L"new");
    {
        SetProperty(AMF_DEVICE_NAME, AMFVariant(GetCLDeviceNameByID(m_deviceID).c_str()));
    }

    {
        cl_uint align = 0;

        if(CL_SUCCESS == clGetDeviceInfo(m_deviceID, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(align), &align, NULL))
        {
            if(align > 0)
            {
                SetProperty(AMF_DEVICE_BASEMEMORY_ALIGN, AMFVariant(align));
            }
        }
    }
    //todo: support this properties too
    //(only if device is AMD based)
    /*{
        cl_device_topology_amd pciBusInfo = {0};
        cl_int status = clGetDeviceInfo(deviceID, CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &pciBusInfo, nullptr);

        if(AMFResultIsOK(status))
        {
            fprintf(stdout, "PCI bus: %d device: %d function: %d\n", pciBusInfo.pcie.bus, pciBusInfo.pcie.device, pciBusInfo.pcie.function);
        }
    }

    {
        cl_uint max_CUs = 0;
        cl_int status = clGetDeviceInfo(deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_CUs, nullptr);

        if(AMFResultIsOK(status))
        {
            fprintf(stdout, "max compute units: %d\n", max_CUs);
        }
    }*/
}

AMFComputeDeviceOCLImpl::~AMFComputeDeviceOCLImpl()
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeDeviceOCLImpl", 0, L"delete");
    clReleaseDevice(m_deviceID);
    clReleaseContext(m_context);
}

void* AMFComputeDeviceOCLImpl::GetNativePlatform()
{
    return m_platformID;
}

void* AMFComputeDeviceOCLImpl::GetNativeDeviceID()
{
	return m_deviceID;
}

void* AMFComputeDeviceOCLImpl::GetNativeContext()
{
    return m_context;
}

AMF_RESULT AMFComputeDeviceOCLImpl::CreateCompute(void* reserved, AMFCompute** ppCompute)
{
    cl_int status = 0;
    cl_command_queue commandQueue = nullptr;

#if CL_TARGET_OPENCL_VERSION >= 200
    commandQueue = clCreateCommandQueueWithProperties(m_context, m_deviceID, (cl_command_queue_properties)NULL, &status);
#else
    commandQueue = clCreateCommandQueue(m_context, m_deviceID, NULL, &status);
#endif

    if (!commandQueue)
    {
        printf("Error: Failed to create a commands Queue!\n");
        return AMF_FAIL;
    }
    AMFComputeOCLImpl* computeOCL = new AMFComputeOCLImpl(m_platformID, m_deviceID, m_pContext, m_context, commandQueue);
    *ppCompute = computeOCL;
    (*ppCompute)->Acquire();
    return AMF_OK;
}

AMF_RESULT AMFComputeDeviceOCLImpl::CreateComputeEx(void* pCommandQueue, AMFCompute** ppCompute)
{
    AMFComputeOCLImpl* computeOCL = new AMFComputeOCLImpl(m_platformID, m_deviceID, m_pContext, m_context, (cl_command_queue)pCommandQueue);
    *ppCompute = computeOCL;
    (*ppCompute)->Acquire();
    return AMF_OK;
}

AMFDeviceImpl* AMFComputeDeviceOCLImpl::GetDevice() const
{
    return m_device;
}

AMFDeviceOCLImpl::AMFDeviceOCLImpl(cl_platform_id platformID, cl_device_id deviceID, AMFContextImpl* pContext, cl_context context, cl_command_queue command_queue)
    : AMFDeviceImpl(AMF_MEMORY_OPENCL, 0, pContext), m_command_queue(command_queue)
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFDeviceOCLImpl", 0, L"new");
    m_computeDevice = new AMFComputeDeviceOCLImpl(this, m_pContext, platformID, deviceID, context);
    if (!command_queue)
    {
        cl_int status = 0;
        cl_command_queue commandQueue = nullptr;

#if CL_TARGET_OPENCL_VERSION >= 200
        commandQueue = clCreateCommandQueueWithProperties(context, deviceID, (cl_command_queue_properties)NULL, &status);
#else
        commandQueue = clCreateCommandQueue(context, deviceID, NULL, &status);
#endif

        if (!commandQueue)
        {
            printf("Error: Failed to create a commands Queue!\n");
        }
        else
            m_command_queue = commandQueue;
    }
}

AMFDeviceOCLImpl::~AMFDeviceOCLImpl()
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFDeviceOCLImpl", 0, L"delete");
    m_computeDevice = nullptr;
    clReleaseCommandQueue(m_command_queue);
}

AMF_RESULT AMFDeviceOCLImpl::AllocateBuffer(amf_size size, void** ppHandle)
{
    return AllocateBufferEx(size, ppHandle, AMF_ARGUMENT_ACCESS_READWRITE);
}

AMF_RESULT AMFDeviceOCLImpl::AttachBuffer(amf_size size, void* pHandle)
{
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::ReleaseBuffer(void* pHandle, bool attached)
{
    if (!attached)
    {
        int err = clReleaseMemObject((cl_mem)pHandle);
        if (err != CL_SUCCESS)
        {
            printf("Error: clReleaseMemObject failed! Invalid object.");
            return AMF_FAIL;
        }
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::CreateSubBuffer(AMFBuffer* pHandle, void** subBuffer, amf_size offset, amf_size size)
{
    AMF_RETURN_IF_FALSE(subBuffer != 0, AMF_INVALID_ARG, L"AllocateBufferEx() - ppHandle cannot be 0");

    //TODO: check type
    cl_buffer_region region;
    region.origin = offset;
    region.size = size;
    int err = 0;
    (*subBuffer) = clCreateSubBuffer(
        (cl_mem)pHandle->GetNative(),
        CL_MEM_READ_WRITE,
        CL_BUFFER_CREATE_TYPE_REGION,
        &region,
        &err
        );

    AMF_RETURN_IF_CL_FAILED(err);

    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::MapToHost(AMFBuffer* pHandle, void** memory, amf_size offset, amf_size size, bool blocking)
{
    //TODO:: check type
    int err = CL_SUCCESS;
    (*memory) = clEnqueueMapBuffer(m_command_queue, (cl_mem)pHandle->GetNative(), blocking, CL_MAP_READ, offset, size, 0, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to clEnqueueMapBuffer! Code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::CopyBuffer(void* pDestHandle, amf_size dstOffset, void* pSourceHandle, amf_size srcOffset, amf_size size)
{
    int err = clEnqueueCopyBuffer(m_command_queue, (cl_mem)pSourceHandle, (cl_mem)pDestHandle, srcOffset, dstOffset, size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to clEnqueueCopyBuffer! Code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::CopyBufferToHost(void* pDest, void* pSourceHandle, amf_size srcOffset, amf_size size, bool blocking)
{
    int err = clEnqueueReadBuffer(m_command_queue, (cl_mem)pSourceHandle, (blocking) ? CL_TRUE : CL_FALSE, srcOffset, size, (float*)pDest, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to clEnqueueReadBuffer! Code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::CopyBufferFromHost(void* pDestHandle, amf_size dstOffset, const void* pSource, amf_size size, bool blocking)
{
    int err = clEnqueueWriteBuffer(m_command_queue, (cl_mem)pDestHandle, (blocking) ? CL_TRUE : CL_FALSE, dstOffset, size, (const float*)pSource, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to clEnqueueWriteBuffer! Code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::FillBuffer(void* pDestHandle, amf_size dstOffset, amf_size dstSize, const void* pSourcePattern, amf_size patternSize)
{
    int err = clEnqueueFillBuffer(m_command_queue, (cl_mem)pDestHandle, pSourcePattern, patternSize, dstOffset, dstSize, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to clEnqueueFillBuffer! Code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFDeviceOCLImpl::AllocateBufferEx(amf_size size, void** ppHandle, AMF_ARGUMENT_ACCESS_TYPE format)
{
    AMF_RETURN_IF_FALSE(size != 0, AMF_INVALID_ARG, L"AllocateBufferEx() - size cannot be 0");
    AMF_RETURN_IF_FALSE(ppHandle != 0, AMF_INVALID_ARG, L"AllocateBufferEx() - ppHandle cannot be 0");

    cl_int returnCode(CL_SUCCESS);
    *ppHandle = clCreateBuffer(
        (cl_context)m_computeDevice->GetNativeContext(),
        amf_to_cl_format(format),
        size,
        NULL,
        &returnCode
        );

    AMF_RETURN_IF_CL_FAILED(returnCode, L"Error: AllocateBuffer failed!\n");
    AMF_RETURN_IF_FALSE(*ppHandle != nullptr, AMF_OUT_OF_MEMORY, L"Error: Failed to allocate device memory!\n");


    return AMF_OK;
}

void* AMFDeviceOCLImpl::GetNativeCommandQueue()
{
	return m_command_queue;
}

const AMFComputeDevicePtr & AMFDeviceOCLImpl::GetComputeDevice() const
{
	return m_computeDevice;
}

AMFComputeOCLImpl::AMFComputeOCLImpl(cl_platform_id platformID, cl_device_id deviceID, AMFContextImpl* pContext, cl_context context, cl_command_queue command_queue)
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeOCLImpl", 0, L"new");
    m_device = new AMFDeviceOCLImpl(platformID, deviceID, pContext, context, command_queue);
}

AMFComputeOCLImpl::~AMFComputeOCLImpl()
{
    amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeOCLImpl", 0, L"delete");
    delete m_device;
}

AMF_MEMORY_TYPE AMFComputeOCLImpl::GetMemoryType()
{
    return m_device->GetType();
}

void* AMFComputeOCLImpl::GetNativeContext()
{
    return m_device->GetComputeDevice()->GetNativeContext();
}

void* AMFComputeOCLImpl::GetNativeDeviceID()
{
    return m_device->GetComputeDevice()->GetNativeDeviceID();
}

void* AMFComputeOCLImpl::GetNativeCommandQueue()
{
    return m_device->GetNativeCommandQueue();
}

AMF_RESULT AMFComputeOCLImpl::GetKernel(AMF_KERNEL_ID kernelID, AMFComputeKernel** kernel)
{
    AMFKernelStorage::KernelData* kernelData;
    AMF_RESULT res;
    cl_kernel kernel_CL;
    int err;

    res = AMFKernelStorage::Instance()->GetKernelData(&kernelData, kernelID);
    if (res != AMF_OK) {
        return res;
    }
    cl_program program;

    AMFVariant deviceName;
    AMF_RETURN_IF_FAILED(m_device->GetComputeDevice()->GetProperty(AMF_DEVICE_NAME, &deviceName));

    if (kernelData->type == AMFKernelStorage::KernelData::Source)
    {
        AMFKernelStorage::KernelData* cacheKernelData(nullptr);
        res = AMFKernelStorage::Instance()->GetCacheKernelData(
            &cacheKernelData,
            kernelData->kernelid_name.c_str(),
            kernelData->kernelName.c_str(),
            deviceName.ToString().c_str(),
            ".cl.bin"
            );

        if (res == AMF_OK)
            kernelData = cacheKernelData;//now kernelData->type == AMFKernelStorage::KernelData::Binary
    }

    if (kernelData->type == AMFKernelStorage::KernelData::Binary)
    {
        program = CreateProgramFromBinary(
            (cl_context)GetNativeContext(),
            (cl_device_id)GetNativeDeviceID(),
            kernelData->data
            );

        if (program)
        {
            kernel_CL = clCreateKernel(program, kernelData->kernelName.c_str(), &err);
            if (!kernel_CL || err != CL_SUCCESS)
            {
                printf("Error: Failed to create compute kernel!\n");
                return AMF_FAIL;
            }

            AMFComputeKernelOCL* computeKernel = new AMFComputeKernelOCL(program, kernel_CL, (cl_command_queue)GetNativeCommandQueue(),
				kernelID, (cl_device_id)GetNativeDeviceID(), (cl_context)GetNativeContext());
            *kernel = computeKernel;
            (*kernel)->Acquire();
            return AMF_OK;
        }
        else
        {
            printf("Error: Failed to create compute program from binary!\n");
            amf_int64 sourceKernelId = AMFKernelStorage::Instance()->FindSourceIndex(kernelData->kernelid_name.c_str(), kernelData->options.c_str());
            if (sourceKernelId < 0)
            {
                printf("Error: Failed to find kernel source!\n");
                return AMF_FAIL;
            }
            res = AMFKernelStorage::Instance()->GetKernelData(&kernelData, sourceKernelId);
            if (res != AMF_OK) {
                printf("Error: Failed to fetch kernel source!\n");
                return res;
            }
        }
    }
    else
    {
        std::string source(reinterpret_cast<const char *>(&kernelData->data.front()), kernelData->data.size());
		const char *pointer(source.c_str());
        program = clCreateProgramWithSource((cl_context)GetNativeContext(), 1, &pointer, nullptr, &err);
        
        if (!program || err != CL_SUCCESS)
        {
            printf("Error: Failed to create compute program!\n");
            return AMF_FAIL;
        }

        err = clBuildProgram(program, 0, NULL, kernelData->options.c_str(), NULL, NULL);
        if (err != CL_SUCCESS)
        {
            size_t len;
            char buffer[2048];
            printf("Error: Failed to build program executable!\n");
            clGetProgramBuildInfo(program, (cl_device_id)GetNativeDeviceID(), CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            printf("%s\n", buffer);
            return AMF_FAIL;
        }

        if (AMFKernelStorage::Instance()->GetCacheFolder().length())
            SaveProgramBinary(program, (cl_device_id)GetNativeDeviceID(), kernelData->kernelid_name.c_str(), kernelData->kernelName.c_str());

        kernel_CL = clCreateKernel(program, kernelData->kernelName.c_str(), &err);
        if (!kernel_CL || err != CL_SUCCESS)
        {
            printf("Error: Failed to create compute kernel!\n");
            return AMF_FAIL;
        }
    }

    AMFComputeKernelOCL* computeKernel = new AMFComputeKernelOCL(
        program,
        kernel_CL,
        (cl_command_queue)GetNativeCommandQueue(),
        kernelID,
		(cl_device_id)GetNativeDeviceID(),
        (cl_context)GetNativeContext()
        );
    *kernel = computeKernel;
    (*kernel)->Acquire();
	m_kernels.push_back(computeKernel);

    return AMF_OK;
}

AMF_RESULT AMFComputeOCLImpl::PutSyncPoint(AMFComputeSyncPoint** ppSyncPoint)
{
	AMFComputeSyncPointOCLImpl * result = new AMFComputeSyncPointOCLImpl(this);
	*ppSyncPoint = result;
	(*ppSyncPoint)->Acquire();
	return AMF_OK;
}

AMF_RESULT AMFComputeOCLImpl::FinishQueue()
{
    int err = 0;
    err = clFinish((cl_command_queue)m_device->GetNativeCommandQueue());
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to FinishQueue! Error code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeOCLImpl::FlushQueue()
{
    int err = 0;
    err = clFlush((cl_command_queue)m_device->GetNativeCommandQueue());
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to Flush Queue! Error code = %d\n", err);
        return AMF_FAIL;
    }
    return AMF_OK;
}

AMF_RESULT AMFComputeOCLImpl::FillPlane(AMFPlane* pPlane, const amf_size origin[], const amf_size region[], const void* pColor)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeOCLImpl::FillBuffer(AMFBuffer* pBuffer, amf_size dstOffset, amf_size dstSize, const void* pSourcePattern, amf_size patternSize)
{
    //TODO: check buffer type
    return m_device->FillBuffer(pBuffer->GetNative(), dstOffset, dstSize, pSourcePattern, patternSize);
}

AMF_RESULT AMFComputeOCLImpl::ConvertPlaneToBuffer(AMFPlane* pSrcPlane, AMFBuffer** ppDstBuffer)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeOCLImpl::CopyBuffer(AMFBuffer* pSrcBuffer, amf_size srcOffset, amf_size size, AMFBuffer* pDstBuffer, amf_size dstOffset)
{
    return m_device->CopyBuffer(pDstBuffer->GetNative(), dstOffset, pSrcBuffer->GetNative(), srcOffset, size);
}

AMF_RESULT AMFComputeOCLImpl::CopyPlane(AMFPlane* pSrcPlane, const amf_size srcOrigin[], const amf_size region[], AMFPlane* pDstPlane, const amf_size dstOrigin[])
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeOCLImpl::CopyBufferToHost(AMFBuffer* pSrcBuffer, amf_size srcOffset, amf_size size, void* pDest, amf_bool blocking)
{
    //TODO: memory type check
    return m_device->CopyBufferToHost(pDest, pSrcBuffer->GetNative(), srcOffset, size, blocking);
}

AMF_RESULT AMFComputeOCLImpl::CopyBufferFromHost(const void* pSource, amf_size size, AMFBuffer* pDstBuffer, amf_size dstOffsetInBytes, amf_bool blocking)
{
    //TODO: memory type check
    return m_device->CopyBufferFromHost(pDstBuffer->GetNative(), dstOffsetInBytes, pSource, size, blocking);
}

AMF_RESULT AMFComputeOCLImpl::CopyPlaneToHost(AMFPlane* pSrcPlane, const amf_size origin[], const amf_size region[], void* pDest, amf_size dstPitch, amf_bool blocking)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeOCLImpl::CopyPlaneFromHost(void* pSource, const amf_size origin[], const amf_size region[], amf_size srcPitch, AMFPlane* pDstPlane, amf_bool blocking)
{
    return AMF_NOT_IMPLEMENTED;
}

AMF_RESULT AMFComputeOCLImpl::ConvertPlaneToPlane(AMFPlane* pSrcPlane, AMFPlane** ppDstPlane, AMF_CHANNEL_ORDER order, AMF_CHANNEL_TYPE type)
{
    return AMF_NOT_IMPLEMENTED;
}

amf_vector<cl_event> AMFComputeOCLImpl::GetKernelEvents()
{
	amf_vector<cl_event> result;
	for (int i = 0; i < m_kernels.size(); i++)
	{
		AMFComputeKernelOCL * kernel = reinterpret_cast<AMFComputeKernelOCL *>(m_kernels.at(i).GetPtr());
		result.push_back(kernel->GetEvent());
	}
	return result;
}

AMFComputeSyncPointOCLImpl::AMFComputeSyncPointOCLImpl(AMFComputeOCLImpl * compute) : m_compute(compute)
{
	int ret = 0;
	m_event = clCreateUserEvent((cl_context)m_compute->GetNativeContext(), &ret);
	if (ret != CL_SUCCESS)
	{
		amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeSyncPointOCLImpl", 0, L"failed to create event");
	}
	amf_vector<cl_event> events = m_compute->GetKernelEvents();
	
	ret = clEnqueueMarkerWithWaitList(cl_command_queue(	m_compute->GetNativeCommandQueue()),
														events.size(),
														events.data(),
														&m_event);
	if (ret != CL_SUCCESS)
	{
		amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeSyncPointOCLImpl", 0, L"failed to clEnqueueMarkerWithWaitList");
	}
}

AMFComputeSyncPointOCLImpl::~AMFComputeSyncPointOCLImpl()
{
	clReleaseEvent(m_event);
}

amf_bool AMFComputeSyncPointOCLImpl::IsCompleted()
{
	cl_int ret = 0;
	cl_int result = 0;
	ret = clGetEventInfo(m_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
	sizeof(result), &result, NULL);
	if ((ret == CL_SUCCESS) && (result == CL_COMPLETE))
		return true;
	return false;
}

void AMFComputeSyncPointOCLImpl::Wait()
{
	cl_int ret = 0;
	ret = clWaitForEvents(1, &m_event);
	if (ret != CL_SUCCESS)
	{
		amf::AMFTraceW(AMF_UNICODE(__FILE__), __LINE__, AMF_TRACE_TEST, L"AMFComputeSyncPointOCLImpl", 0, L"failed to clWaitForEvents");
	}
}