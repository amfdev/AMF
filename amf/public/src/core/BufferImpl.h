#ifndef BUFFERIMPL_H
#define BUFFERIMPL_H

#include "../../include/core/Buffer.h"
#include "../../common/ObservableImpl.h"
#include "DataImpl.h"
#include "Device.h"

using namespace amf;

class AMFBufferImpl : public AMFDataImpl< AMFInterfaceImpl<AMFBuffer> >, public AMFObservableImpl<AMFBufferObserver>
{
public:
    AMFBufferImpl(AMFContextImpl* pContext);

    AMF_BEGIN_INTERFACE_MAP
        AMF_INTERFACE_CHAIN_ENTRY(AMFInterfaceImpl)
        AMF_INTERFACE_CHAIN_ENTRY(AMFDataImpl)
    AMF_END_INTERFACE_MAP

    // AMFData interface
public:
    virtual AMF_MEMORY_TYPE AMF_STD_CALL GetMemoryType() override;
    virtual AMF_RESULT AMF_STD_CALL Duplicate(AMF_MEMORY_TYPE type, AMFData **ppData) override;
    virtual AMF_RESULT AMF_STD_CALL Convert(AMF_MEMORY_TYPE type) override;
    virtual AMF_RESULT AMF_STD_CALL Interop(AMF_MEMORY_TYPE type) override;
    virtual AMF_DATA_TYPE AMF_STD_CALL GetDataType() override;

    // AMFBuffer interface
public:
    virtual AMF_RESULT AMF_STD_CALL SetSize(amf_size newSize) override;
    virtual amf_size AMF_STD_CALL GetSize() override;

    virtual void *AMF_STD_CALL GetNative() override;
	virtual AMF_RESULT AMF_STD_CALL CreateSubBuffer(AMFBuffer** subBuffer, amf_size offset, amf_size size) override;

    virtual void AMF_STD_CALL AddObserver(AMFPropertyStorageObserver* pObserver) override { AMFObservableImpl<AMFPropertyStorageObserver>::AddObserver(pObserver); }
    virtual void AMF_STD_CALL RemoveObserver(AMFPropertyStorageObserver* pObserver) override { AMFObservableImpl<AMFPropertyStorageObserver>::RemoveObserver(pObserver); }

    virtual void AMF_STD_CALL AddObserver(AMFBufferObserver *pObserver) override { AMFObservableImpl<AMFBufferObserver>::AddObserver(pObserver); }
    virtual void AMF_STD_CALL RemoveObserver(AMFBufferObserver *pObserver) override { AMFObservableImpl<AMFBufferObserver>::RemoveObserver(pObserver); }
public:
    virtual AMF_RESULT AMF_STD_CALL Allocate(AMF_MEMORY_TYPE type, amf_size size);
    virtual AMF_RESULT AMF_STD_CALL Attach(AMF_MEMORY_TYPE type, void* pNative, amf_size size);

    virtual AMF_RESULT AMF_STD_CALL Duplicate(AMF_MEMORY_TYPE type, AMFBufferImpl **ppData);
private:
    void *m_pMemory = nullptr;
    amf_size m_size = 0;
    bool m_attached = false;

    AMFDevice *m_pDevice = nullptr;
};

#endif // BUFFERIMPL_H