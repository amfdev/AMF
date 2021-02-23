#ifndef AMFTRACEIMPL_H
#define AMFTRACEIMPL_H
#include "../../include/core/Trace.h"
#include "../../include/core/Data.h"
#include "AMFFileTraceWriter.h"
#include "AMFConsoleTraceWriter.h"
#include <map>
#include <functional>
#include <cstring>

#include <iostream>

using namespace amf;

struct WStrCompare : public std::binary_function<const wchar_t*, const wchar_t*, bool> {
public:
	bool operator() (const wchar_t* str1, const wchar_t* str2) const
	{
		return std::wcscmp(str1, str2) < 0;
	}
};

class AMFTraceImpl : public AMFTrace
{
public:
    AMFTraceImpl();

	// AMFTrace interface
public:
    virtual  void               AMF_STD_CALL TraceW(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope,amf_int32 countArgs, const wchar_t* format, ...);
    virtual  void               AMF_STD_CALL Trace(const wchar_t* src_path, amf_int32 line, amf_int32 level, const wchar_t* scope, const wchar_t* message, va_list* pArglist);

    virtual amf_int32           AMF_STD_CALL SetGlobalLevel(amf_int32 level);
    virtual amf_int32           AMF_STD_CALL GetGlobalLevel();

    virtual amf_bool            AMF_STD_CALL EnableWriter(const wchar_t* writerID, bool enable);
    virtual amf_bool            AMF_STD_CALL WriterEnabled(const wchar_t* writerID);
    virtual AMF_RESULT          AMF_STD_CALL TraceEnableAsync(amf_bool enable);
    virtual AMF_RESULT          AMF_STD_CALL TraceFlush();
    virtual AMF_RESULT          AMF_STD_CALL SetPath(const wchar_t* path);
    virtual AMF_RESULT          AMF_STD_CALL GetPath(wchar_t* path, amf_size* pSize);
    virtual amf_int32           AMF_STD_CALL SetWriterLevel(const wchar_t* writerID, amf_int32 level);
    virtual amf_int32           AMF_STD_CALL GetWriterLevel(const wchar_t* writerID);
    virtual amf_int32           AMF_STD_CALL SetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope, amf_int32 level);
    virtual amf_int32           AMF_STD_CALL GetWriterLevelForScope(const wchar_t* writerID, const wchar_t* scope);

    virtual amf_int32           AMF_STD_CALL GetIndentation();
    virtual void                AMF_STD_CALL Indent(amf_int32 addIndent);

    virtual void                AMF_STD_CALL RegisterWriter(const wchar_t* writerID, AMFTraceWriter* pWriter, amf_bool enable);
    virtual void                AMF_STD_CALL UnregisterWriter(const wchar_t* writerID);

    virtual const wchar_t*      AMF_STD_CALL GetResultText(AMF_RESULT res);
    virtual const wchar_t*      AMF_STD_CALL SurfaceGetFormatName(const AMF_SURFACE_FORMAT eSurfaceFormat);
    virtual AMF_SURFACE_FORMAT  AMF_STD_CALL SurfaceGetFormatByName(const wchar_t* name);

    virtual const wchar_t* const AMF_STD_CALL GetMemoryTypeName(const AMF_MEMORY_TYPE memoryType);
    virtual AMF_MEMORY_TYPE     AMF_STD_CALL GetMemoryTypeByName(const wchar_t* name);

    virtual const wchar_t* const AMF_STD_CALL GetSampleFormatName(const AMF_AUDIO_FORMAT eFormat);
    virtual AMF_AUDIO_FORMAT    AMF_STD_CALL GetSampleFormatByName(const wchar_t* name);

private:
	struct AMFTraceWriterEx
	{
		AMFTraceWriter* pWriter;
		const wchar_t* writerID;
		int level;
		bool enable;
		std::map<const wchar_t*, amf_int32, WStrCompare> levels;
	};
private:
	std::wstring GetWriterLevelName(amf_int32 level);
	std::wstring FormMessage(amf_int32 level, const wchar_t *srcPath, amf_int32 line, const wchar_t *message, va_list *pArglist);
	bool CheckLevel(int current, int traceLevel);
	AMFTraceWriterEx * WriterByName(const wchar_t* name);

	AMFFileTraceWriter m_fileWriter;
	AMFConsoleTraceWriter m_consoleWriter;
	AMFTraceWriterEx m_consoleEx;

	std::map<const wchar_t*, AMFTraceWriterEx, WStrCompare> m_writers;

	amf_int32 m_level;
	amf_int32 m_indent;
};

#endif // AMFTRACEIMPL_H
