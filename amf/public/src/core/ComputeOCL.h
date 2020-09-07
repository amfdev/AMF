#ifndef AMFCOMPUTEOCL_H
#define AMFCOMPUTEOCL_H

#include "../../include/core/Compute.h"
#include "../../include/core/ComputeFactory.h"
#include "../../common/InterfaceImpl.h"
#include "../../common/AMFSTL.h"
#include "../../common/TraceAdapter.h"
#include "../../common/PropertyStorageImpl.h"
#include <CL/cl.h>
#include "DeviceOCLImpl.h"

using namespace amf;

class AMFComputeKernelOCL : public AMFInterfaceImpl<AMFComputeKernel>
{
public:
    AMFComputeKernelOCL(cl_program program, cl_kernel kernel, cl_command_queue command_queue, AMF_KERNEL_ID kernelID, cl_device_id deviceID, cl_context context);
	~AMFComputeKernelOCL();
    virtual void*               AMF_STD_CALL GetNative();
    virtual const wchar_t*      AMF_STD_CALL GetIDName();

    virtual AMF_RESULT          AMF_STD_CALL SetArgPlaneNative(amf_size index, void* pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess);
    virtual AMF_RESULT          AMF_STD_CALL SetArgBufferNative(amf_size index, void* pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess);

    virtual AMF_RESULT          AMF_STD_CALL SetArgPlane(amf_size index, AMFPlane* pPlane, AMF_ARGUMENT_ACCESS_TYPE eAccess);
    virtual AMF_RESULT          AMF_STD_CALL SetArgBuffer(amf_size index, AMFBuffer* pBuffer, AMF_ARGUMENT_ACCESS_TYPE eAccess);

    virtual AMF_RESULT          AMF_STD_CALL SetArgInt32(amf_size index, amf_int32 data);
    virtual AMF_RESULT          AMF_STD_CALL SetArgInt64(amf_size index, amf_int64 data);
    virtual AMF_RESULT          AMF_STD_CALL SetArgFloat(amf_size index, amf_float data);
    virtual AMF_RESULT          AMF_STD_CALL SetArgBlob(amf_size index, amf_size dataSize, const void* pData);

    virtual AMF_RESULT          AMF_STD_CALL GetCompileWorkgroupSize(amf_size workgroupSize[3]);

    virtual AMF_RESULT          AMF_STD_CALL Enqueue(amf_size dimension, amf_size globalOffset[3], amf_size globalSize[3], amf_size localSize[3]);
	cl_event					GetEvent();
private:
    AMF_KERNEL_ID m_kernelID;
    cl_kernel m_kernel;
    cl_context m_context;
    cl_program m_program;
    cl_command_queue m_command_queue;
    cl_device_id m_deviceID;
	cl_event m_event;
};

class AMFComputeFactoryOCL : public AMFInterfaceImpl<AMFComputeFactory>
{
public:
    AMFComputeFactoryOCL(AMFContextImpl* pContext):m_pContext(pContext){}
	virtual ~AMFComputeFactoryOCL() {}
    AMF_RESULT Init();
    virtual amf_int32           AMF_STD_CALL GetDeviceCount() override;
    virtual AMF_RESULT          AMF_STD_CALL GetDeviceAt(amf_int32 index, AMFComputeDevice **ppDevice) override;
private:
    amf_vector<AMFDevicePtr> m_devices;
    AMFContextImpl* m_pContext;
};
#endif // AMFCOMPUTEOCL_H
