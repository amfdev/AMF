#include "../MetalDeviceWrapper.h"

const unsigned int arrayLength = 1 << 12;
const unsigned int bufferSize = arrayLength * sizeof(float);

int main(int argc, char *argv[])
{
    MetalDeviceWrapper * device = new MetalDeviceWrapper(NULL);
	AMF_RESULT res;

    MetalComputeWrapper * compute = NULL;
    res = device->CreateCompute(&compute);

    MetalComputeKernelWrapper * kernel = NULL;
    res = compute->GetKernel("process_array", &kernel);

    void * input = device->AllocateBuffer(bufferSize);
    void *  output = device->AllocateBuffer(bufferSize);

    float data [arrayLength];
    float result [arrayLength];

    for (unsigned long index = 0; index < arrayLength; index++)
    {
        data[index] = (float)rand()/(float)(RAND_MAX);
        result[index] = 0;
    }

    res = device->CopyBufferFromHost(input, 0, &data[0], bufferSize, true);
printf("Hello");
    res = kernel->SetArgBuffer(input, 0);
    res = kernel->SetArgBuffer(output, 1);
printf("Hello");
    amf_size globalSize[3] = {arrayLength, 1, 1};
    amf_size localSize[3] = {arrayLength, 1, 1};

    res = kernel->GetCompileWorkgroupSize(localSize);
    res = kernel->Enqueue(globalSize, localSize);

printf("Hello");
    res = compute->FlushQueue();
printf("Hello");
    res = compute->FinishQueue();
printf("Hello");
    res = device->CopyBufferToHost(&result[0], output, 0, bufferSize, true);

    for (unsigned long index = 0; index < 10; index++)
    {
        printf("result[%d] = %f", index, result[index]);
    }

    return 0;
}
