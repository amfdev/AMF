#include "../../include/core/Factory.h"
#include "../../common/AMFFactory.h"
#include "../../include/core/Buffer.h"

#define CL_TARGET_OPENCL_VERSION 120

int main(int argc, char *argv[])
{
    AMFFactoryHelper helper;
    helper.Init();
    amf::AMFFactory* factory = helper.GetFactory();

    amf::AMFContextPtr context1;
    factory->CreateContext(&context1);

	context1->SetProperty(AMF_CONTEXT_DEVICE_TYPE, AMF_CONTEXT_DEVICE_TYPE_GPU);
    amf::AMFComputeFactoryPtr oclComputeFactory;
    context1->GetOpenCLComputeFactory(&oclComputeFactory);
	context1->InitOpenCL();

    amf::AMFPrograms* pPrograms;
    factory->GetPrograms(&pPrograms);

    amf::AMF_KERNEL_ID kernel = 0;
    amf::AMF_KERNEL_ID kernel2 = 0;
    
    const char* kernel_src = "\n" \
                             "__kernel void square( __global float* input, __global float* output, \n" \
                             " const unsigned int count) {            \n" \
                             " int i = get_global_id(0);              \n" \
                             " if(i < count) \n" \
                             " output[i] = -input[i]; \n" \
                             "}                     \n"
		"\n" \
		"__kernel void square2( __global float* input, __global float* output, \n" \
		" const unsigned int count) {            \n" \
		" int i = get_global_id(0);              \n" \
		" if(i < count) \n" \
		" output[i] = input[i] * input[i]; \n" \
		"}                     \n";
    pPrograms->RegisterKernelSource(&kernel, L"kernelIDName", "square2", strlen(kernel_src), (amf_uint8*)kernel_src, NULL);
    pPrograms->RegisterKernelSource(&kernel, L"kernelIDName", "square2", strlen(kernel_src), (amf_uint8*)kernel_src, NULL);
    pPrograms->RegisterKernelSource(&kernel2, L"kernelIDName2", "square2", strlen(kernel_src), (amf_uint8*)kernel_src, NULL);

    int deviceCount = oclComputeFactory->GetDeviceCount();
	g_AMFFactory.Init();
    g_AMFFactory.GetDebug()->AssertsEnable(true);


	g_AMFFactory.GetTrace()->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_TEST);
	g_AMFFactory.GetTrace()->SetGlobalLevel(AMF_TRACE_TEST);
	g_AMFFactory.GetTrace()->SetWriterLevel(AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_TRACE);
	g_AMFFactory.GetTrace()->SetWriterLevelForScope(AMF_TRACE_WRITER_CONSOLE, L"scope2", AMF_TRACE_TRACE);
	g_AMFFactory.GetTrace()->SetWriterLevelForScope(AMF_TRACE_WRITER_CONSOLE, L"scope2", AMF_TRACE_ERROR);

    g_AMFFactory.GetTrace()->SetGlobalLevel(AMF_TRACE_TEST);
	g_AMFFactory.GetTrace()->SetPath(L"E:\\tmp\\openamftest.log");
	g_AMFFactory.GetTrace()->EnableWriter(AMF_TRACE_WRITER_FILE, true);
	g_AMFFactory.GetTrace()->TraceW(L"path", 387, AMF_TRACE_ERROR, L"scope", 4, L"mesage1");
	g_AMFFactory.GetTrace()->Indent(8);
	g_AMFFactory.GetTrace()->TraceW(L"path", 387, AMF_TRACE_INFO, L"scope", 4, L"mesage2");
	g_AMFFactory.GetTrace()->Indent(-4);
	g_AMFFactory.GetTrace()->TraceW(L"path", 387, AMF_TRACE_DEBUG, L"scope", 4, L"mesage3(%d)", 4);
	g_AMFFactory.GetTrace()->TraceW(L"path", 387, AMF_TRACE_DEBUG, L"scope2", 4, L"mesage3(%d)", 4);

	g_AMFFactory.GetTrace()->Trace(L"", 11, AMF_TRACE_WARNING, L"scope", L"message4", nullptr);

#ifdef WIN32
	g_AMFFactory.GetFactory()->SetCacheFolder(L"amf_cache");
#else
    //not a full path will not work on macosx
    //please see https://stackoverflow.com/questions/23974831/problems-with-fopen-function-on-mac
	g_AMFFactory.GetFactory()->SetCacheFolder(L"/Users/user/test");
#endif

    for(int i = 0; i < deviceCount; ++i)
    {
        AMF_RESULT res;
        amf::AMFComputeDevicePtr pComputeDevice;
        oclComputeFactory->GetDeviceAt(i, &pComputeDevice);
        pComputeDevice->GetNativeContext();

        amf::AMFComputePtr pCompute;
        pComputeDevice->CreateCompute(nullptr, &pCompute);

        amf::AMFComputeKernelPtr pKernel;
        res = pCompute->GetKernel(kernel, &pKernel);
        
        amf::AMFComputeKernelPtr pKernel2;
        res = pCompute->GetKernel(kernel2, &pKernel2);

        amf::AMFBuffer *input = NULL;
        amf::AMFBuffer *output = NULL;

		amf::AMFContextPtr context;
		factory->CreateContext(&context);
		//context->InitOpenCLEx(pComputeDevice.GetPtr());
		context->InitOpenCL(pCompute->GetNativeCommandQueue());

		res = context->AllocBuffer(amf::AMF_MEMORY_HOST, 1024 * sizeof(float), &input);
		res = context->AllocBuffer(amf::AMF_MEMORY_OPENCL, 1024 * sizeof(float), &output);

        float  *inputData = static_cast<float*>(input->GetNative());
        for (int k = 0; k < 1024; k++)
        {
			inputData[k] = rand() / 50.00;;
        }

        input->Convert(amf::AMF_MEMORY_OPENCL);

        res = pKernel->SetArgBuffer(1, output, amf::AMF_ARGUMENT_ACCESS_WRITE);
        res = pKernel->SetArgBuffer(0, input, amf::AMF_ARGUMENT_ACCESS_READ);
        res = pKernel->SetArgInt32(2, 1024);

        amf_size sizeLocal[3] = {1024, 0, 0};
        amf_size sizeGlobal[3] = {1024, 0, 0};
        amf_size offset[3] = {0, 0, 0};

        pKernel->GetCompileWorkgroupSize(sizeLocal);

        pKernel->Enqueue(1, offset, sizeGlobal, sizeLocal);
        pCompute->FlushQueue();
        //pCompute->FinishQueue();
		amf::AMFComputeSyncPointPtr pSync;
		res = pCompute->PutSyncPoint(&pSync);
		printf("\nSync completed = %s\n", pSync->IsCompleted() ? "true" : "false");
		pSync->Wait();
		printf("Sync completed = %s\n", pSync->IsCompleted() ? "true" : "false");

		float  *outputData2 = NULL;
		res = output->MapToHost((void**)&outputData2, 0, 1024 * sizeof(float), true);


        for (int k = 0; k < 1024; k++ )
        {
            printf("result[%d] = %f \n", k, outputData2[k]);
        }

		output->Convert(amf::AMF_MEMORY_HOST);
		float  *outputData = static_cast<float*>(output->GetNative());
    }

    return 0;
}
