#include "TraceImpl.h"
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
amf::AMFCriticalSection      s_writer_cs;

AMFTraceImpl::AMFTraceImpl()
{
	m_level = AMF_TRACE_INFO;
	RegisterWriter(AMF_TRACE_WRITER_FILE, &m_fileWriter, true);
	m_consoleEx = AMFTraceWriterEx { &m_consoleWriter, AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_INFO, true };
}

AMF_AUDIO_FORMAT AMFTraceImpl::GetSampleFormatByName(const wchar_t *name)
{
	if (0 == wcscmp(name, L"UNKNOWN"))
		return AMFAF_UNKNOWN;
	if (0 == wcscmp(name, L"U8"))
		return AMFAF_U8;
	if (0 == wcscmp(name, L"S16"))
		return AMFAF_S16;
	if (0 == wcscmp(name, L"S32"))
		return AMFAF_S32;
	if (0 == wcscmp(name, L"FLT"))
		return AMFAF_FLT;
	if (0 == wcscmp(name, L"DBL"))
		return AMFAF_DBL;
	if (0 == wcscmp(name, L"U8P"))
		return AMFAF_U8P;
	if (0 == wcscmp(name, L"S16P"))
		return AMFAF_S16P;
	if (0 == wcscmp(name, L"S32P"))
		return AMFAF_S32P;
	if (0 == wcscmp(name, L"FLTP"))
		return AMFAF_FLTP;
	if (0 == wcscmp(name, L"DBLP"))
		return AMFAF_DBLP;

	return AMFAF_UNKNOWN;
}

const wchar_t * const AMFTraceImpl::GetSampleFormatName(const AMF_AUDIO_FORMAT eFormat)
{
	switch (eFormat)
	{
	case amf::AMFAF_UNKNOWN:
		return L"UNKNOWN";
	case amf::AMFAF_U8:
		return L"U8";
	case amf::AMFAF_S16:
		return L"S16";
	case amf::AMFAF_S32:
		return L"S32";
	case amf::AMFAF_FLT:
		return L"FLT";
	case amf::AMFAF_DBL:
		return L"DBL";
	case amf::AMFAF_U8P:
		return L"U8P";
	case amf::AMFAF_S16P:
		return L"S16P";
	case amf::AMFAF_S32P:
		return L"S32P";
	case amf::AMFAF_FLTP:
		return L"FLTP";
	case amf::AMFAF_DBLP:
		return L"DBLP";
	}
    return L"AMFAF_UNKNOWN";
}

AMF_MEMORY_TYPE AMFTraceImpl::GetMemoryTypeByName(const wchar_t *name)
{
	if (0 == wcscmp(name, L"UNKNOWN"))
		return AMF_MEMORY_UNKNOWN;
	if (0 == wcscmp(name, L"HOST"))
		return AMF_MEMORY_HOST;
	if (0 == wcscmp(name, L"DX9"))
		return AMF_MEMORY_DX9;
	if (0 == wcscmp(name, L"DX11"))
		return AMF_MEMORY_DX11;
	if (0 == wcscmp(name, L"OPENCL"))
		return AMF_MEMORY_OPENCL;
	if (0 == wcscmp(name, L"OPENGL"))
		return AMF_MEMORY_OPENGL;
	if (0 == wcscmp(name, L"XV"))
		return AMF_MEMORY_XV;
	if (0 == wcscmp(name, L"GRALLOC"))
		return AMF_MEMORY_GRALLOC;
	if (0 == wcscmp(name, L"COMPUTE_FOR_DX9"))
		return AMF_MEMORY_COMPUTE_FOR_DX9;
	if (0 == wcscmp(name, L"COMPUTE_FOR_DX11"))
		return AMF_MEMORY_COMPUTE_FOR_DX11;
	if (0 == wcscmp(name, L"VULKAN"))
		return AMF_MEMORY_VULKAN;

    return AMF_MEMORY_UNKNOWN;
}

const wchar_t * const AMFTraceImpl::GetMemoryTypeName(const AMF_MEMORY_TYPE memoryType)
{
	switch (memoryType)
	{
	case amf::AMF_MEMORY_UNKNOWN:
		return L"UNKNOWN";
	case amf::AMF_MEMORY_HOST:
		return L"HOST";
	case amf::AMF_MEMORY_DX9:
		return L"DX9";
	case amf::AMF_MEMORY_DX11:
		return L"DX11";
	case amf::AMF_MEMORY_OPENCL:
		return L"OPENCL";
	case amf::AMF_MEMORY_OPENGL:
		return L"OPENGL";
	case amf::AMF_MEMORY_XV:
		return L"XV";
	case amf::AMF_MEMORY_GRALLOC:
		return L"GRALLOC";
	case amf::AMF_MEMORY_COMPUTE_FOR_DX9:
		return L"COMPUTE_FOR_DX9";
	case amf::AMF_MEMORY_COMPUTE_FOR_DX11:
		return L"COMPUTE_FOR_DX11";
	case amf::AMF_MEMORY_VULKAN:
		return L"VULKAN";
	case amf::AMF_MEMORY_METAL:
		return L"METAL";
	}
    return L"UNKNOWN";
}

AMF_SURFACE_FORMAT AMFTraceImpl::SurfaceGetFormatByName(const wchar_t *name)
{
	if (0 == wcscmp(name, L"UNKNOWN"))
		return AMF_SURFACE_UNKNOWN;
	if (0 == wcscmp(name, L"NV12"))
		return AMF_SURFACE_NV12;
	if (0 == wcscmp(name, L"YV12"))
		return AMF_SURFACE_YV12;
	if (0 == wcscmp(name, L"BGRA"))
		return AMF_SURFACE_BGRA;
	if (0 == wcscmp(name, L"ARGB"))
		return AMF_SURFACE_ARGB;
	if (0 == wcscmp(name, L"RGBA"))
		return AMF_SURFACE_RGBA;
	if (0 == wcscmp(name, L"GRAY8"))
		return AMF_SURFACE_GRAY8;
	if (0 == wcscmp(name, L"YUV420P"))
		return AMF_SURFACE_YUV420P;
	if (0 == wcscmp(name, L"U8V8"))
		return AMF_SURFACE_U8V8;
	if (0 == wcscmp(name, L"YUY2"))
		return AMF_SURFACE_YUY2;
	if (0 == wcscmp(name, L"P010"))
		return AMF_SURFACE_P010;
	if (0 == wcscmp(name, L"RGBA_F16"))
		return AMF_SURFACE_RGBA_F16;
	if (0 == wcscmp(name, L"UYVY"))
		return AMF_SURFACE_UYVY;

    return AMF_SURFACE_UNKNOWN;
}

const wchar_t *AMFTraceImpl::SurfaceGetFormatName(const AMF_SURFACE_FORMAT eSurfaceFormat)
{
	switch (eSurfaceFormat)
	{
	case amf::AMF_SURFACE_UNKNOWN:
		return L"UNKNOWN";
	case amf::AMF_SURFACE_NV12:
		return L"NV12";
	case amf::AMF_SURFACE_YV12:
		return L"YV12";
	case amf::AMF_SURFACE_BGRA:
		return L"BGRA";
	case amf::AMF_SURFACE_ARGB:
		return L"ARGB";
	case amf::AMF_SURFACE_RGBA:
		return L"RGBA";
	case amf::AMF_SURFACE_GRAY8:
		return L"GRAY8";
	case amf::AMF_SURFACE_YUV420P:
		return L"YUV420P";
	case amf::AMF_SURFACE_U8V8:
		return L"U8V8";
	case amf::AMF_SURFACE_YUY2:
		return L"YUY2";
	case amf::AMF_SURFACE_P010:
		return L"P010";
	case amf::AMF_SURFACE_RGBA_F16:
		return L"RGBA_F16";
	case amf::AMF_SURFACE_UYVY:
		return L"UYVY";
	}
	return L"Unexpected";
}

const wchar_t *AMFTraceImpl::GetResultText(AMF_RESULT res)
{
	switch (res)
	{
	case AMF_OK:
		return L"AMF_OK";
	case AMF_FAIL:
		return L"AMF_FAIL";
	case AMF_UNEXPECTED:
		return L"AMF_UNEXPECTED";
	case AMF_ACCESS_DENIED:
		return L"AMF_ACCESS_DENIED";
	case AMF_INVALID_ARG:
		return L"AMF_INVALID_ARG";
	case AMF_OUT_OF_RANGE:
		return L"AMF_OUT_OF_RANGE";
	case AMF_OUT_OF_MEMORY:
		return L"AMF_OUT_OF_MEMORY";
	case AMF_INVALID_POINTER:
		return L"AMF_INVALID_POINTER";
	case AMF_NO_INTERFACE:
		return L"AMF_NO_INTERFACE";
	case AMF_NOT_IMPLEMENTED:
		return L"AMF_NOT_IMPLEMENTED";
	case AMF_NOT_SUPPORTED:
		return L"AMF_NOT_SUPPORTED";
	case AMF_NOT_FOUND:
		return L"AMF_NOT_FOUND";
	case AMF_ALREADY_INITIALIZED:
		return L"AMF_ALREADY_INITIALIZED";
	case AMF_NOT_INITIALIZED:
		return L"AMF_NOT_INITIALIZED";
	case AMF_INVALID_FORMAT:
		return L"AMF_INVALID_FORMAT";
	case AMF_WRONG_STATE:
		return L"AMF_WRONG_STATE";
	case AMF_FILE_NOT_OPEN:
		return L"AMF_FILE_NOT_OPEN";
	case AMF_NO_DEVICE:
		return L"AMF_NO_DEVICE";
	case AMF_DIRECTX_FAILED:
		return L"AMF_DIRECTX_FAILED";
	case AMF_OPENCL_FAILED:
		return L"AMF_OPENCL_FAILED";
	case AMF_GLX_FAILED:
		return L"AMF_GLX_FAILED";
	case AMF_XV_FAILED:
		return L"AMF_XV_FAILED";
	case AMF_ALSA_FAILED:
		return L"AMF_ALSA_FAILED";
	case AMF_EOF:
		return L"AMF_EOF";
	case AMF_REPEAT:
		return L"AMF_REPEAT";
	case AMF_INPUT_FULL:
		return L"AMF_INPUT_FULL";
	case AMF_RESOLUTION_CHANGED:
		return L"AMF_RESOLUTION_CHANGED";
	case AMF_RESOLUTION_UPDATED:
		return L"AMF_RESOLUTION_UPDATED";
	case AMF_INVALID_DATA_TYPE:
		return L"AMF_INVALID_DATA_TYPE";
	case AMF_INVALID_RESOLUTION:
		return L"AMF_INVALID_RESOLUTION";
	case AMF_CODEC_NOT_SUPPORTED:
		return L"AMF_CODEC_NOT_SUPPORTED";
	case AMF_SURFACE_FORMAT_NOT_SUPPORTED:
		return L"AMF_SURFACE_FORMAT_NOT_SUPPORTED";
	case AMF_SURFACE_MUST_BE_SHARED:
		return L"AMF_SURFACE_MUST_BE_SHARED";
	case AMF_DECODER_NOT_PRESENT:
		return L"AMF_DECODER_NOT_PRESENT";
	case AMF_DECODER_SURFACE_ALLOCATION_FAILED:
		return L"AMF_DECODER_SURFACE_ALLOCATION_FAILED";
	case AMF_DECODER_NO_FREE_SURFACES:
		return L"AMF_DECODER_NO_FREE_SURFACES";
	case AMF_ENCODER_NOT_PRESENT:
		return L"AMF_ENCODER_NOT_PRESENT";
	case AMF_DEM_ERROR:
		return L"AMF_DEM_ERROR";
	case AMF_DEM_PROPERTY_READONLY:
		return L"AMF_DEM_PROPERTY_READONLY";
	case AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED:
		return L"AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED";
	case AMF_DEM_START_ENCODING_FAILED:
		return L"AMF_DEM_START_ENCODING_FAILED";
	case AMF_DEM_QUERY_OUTPUT_FAILED:
		return L"AMF_DEM_QUERY_OUTPUT_FAILED";
	case AMF_TAN_CLIPPING_WAS_REQUIRED:
		return L"AMF_TAN_CLIPPING_WAS_REQUIRED";
	case AMF_TAN_UNSUPPORTED_VERSION:
		return L"AMF_TAN_UNSUPPORTED_VERSION";
	case AMF_NEED_MORE_INPUT:
		return L"AMF_NEED_MORE_INPUT";

	}
    return L"";
}

void AMFTraceImpl::UnregisterWriter(const wchar_t *writerID)
{
	amf::AMFLock lock(&s_writer_cs);
	m_writers.erase(writerID);
}

void AMFTraceImpl::RegisterWriter(const wchar_t *writerID, AMFTraceWriter *pWriter, amf_bool enable)
{
	AMFTraceWriterEx ex{pWriter, writerID, AMF_TRACE_INFO, enable};
	m_writers.insert({ writerID, ex });
}

void AMFTraceImpl::Indent(amf_int32 addIndent)
{
	m_indent += addIndent;
	if (addIndent != 0)
		m_fileWriter.updateIndentBuffer(m_indent);
}

amf_int32 AMFTraceImpl::GetIndentation()
{
    return m_indent;
}

amf_int32 AMFTraceImpl::GetWriterLevelForScope(const wchar_t *writerID, const wchar_t *scope)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	bool result = false;

	if (!!writer)
	{
		std::map <const wchar_t *, amf_int32> ::iterator it;

		it = writer->levels.find(scope);

		if (it == writer->levels.end())
			return 0;

		return it->second;
	}

	return 0;
}

amf_int32 AMFTraceImpl::SetWriterLevelForScope(const wchar_t *writerID, const wchar_t *scope, amf_int32 level)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	amf_int32 result = 0;
	if (!!writer)
	{
		std::map <const wchar_t *, amf_int32> ::iterator it;

		it = writer->levels.find(scope);

		if (it == writer->levels.end())
		{
			writer->levels.insert({ scope, level });
			return result;
		}

		result = it->second;
		it->second = level;
	}

	return result;
}

amf_int32 AMFTraceImpl::GetWriterLevel(const wchar_t *writerID)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	bool result = false;
	if (!!writer)
		return writer->level;

	return 0;
}

amf_int32 AMFTraceImpl::SetWriterLevel(const wchar_t *writerID, amf_int32 level)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	amf_int32 result = 0;
	if (!!writer)
	{
		result = writer->level;
		writer->level = level;
	}
	return result;
}

AMF_RESULT AMFTraceImpl::GetPath(wchar_t *path, amf_size *pSize)
{
    return m_fileWriter.GetPath(path, pSize);
}

AMF_RESULT AMFTraceImpl::SetPath(const wchar_t *path)
{
    return m_fileWriter.setPath(path);
}

AMF_RESULT AMFTraceImpl::TraceFlush()
{
	if (m_consoleEx.enable)
		m_consoleEx.pWriter->Flush();

	std::map<const wchar_t*, AMFTraceWriterEx>::iterator iter;

	for (iter = m_writers.begin(); iter != m_writers.end(); ++iter) {
		if (!iter->second.enable)
			continue;

		iter->second.pWriter->Flush();
	}
	return AMF_OK;
}

AMF_RESULT AMFTraceImpl::TraceEnableAsync(amf_bool enable)
{
    return AMF_NOT_IMPLEMENTED;
}

amf_bool AMFTraceImpl::WriterEnabled(const wchar_t *writerID)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	bool result = false;
	if (!!writer)
		return writer->enable;

	return false;
}

amf_bool AMFTraceImpl::EnableWriter(const wchar_t *writerID, bool enable)
{
	AMFTraceWriterEx * writer = WriterByName(writerID);
	bool result = false;
	if (!!writer)
	{
		result = writer->enable;
		writer->enable = enable;
	}
    return result;
}

AMFTraceImpl::AMFTraceWriterEx * AMFTraceImpl::WriterByName(const wchar_t* name)
{
	if (0 == wcscmp(name, AMF_TRACE_WRITER_CONSOLE))
		return &m_consoleEx;

	std::map <const wchar_t *, AMFTraceWriterEx> ::iterator it;

	it = m_writers.find(name);

	if (it == m_writers.end())
		return nullptr;
	return &(it->second);
}

amf_int32 AMFTraceImpl::GetGlobalLevel()
{
    return m_level;
}

amf_int32 AMFTraceImpl::SetGlobalLevel(amf_int32 level)
{
	amf_int32 prev = m_level;
	m_level = level;
    return prev;
}

std::wstring AMFTraceImpl::GetWriterLevelName(amf_int32 level)
{
	static std::map<amf_int32, std::wstring> levelNames = {
		{1, L"Error: "},
		{2, L"Warning: "},
		{3, L"Info: "},
		{4, L"Debug: "},
		{5, L"Trace: "},
		{6, L"Test: "}
		};

	return levelNames[level];

	assert(false);
	return L"";
}

std::wstring AMFTraceImpl::FormMessage
(
	amf_int32 			level,
	const wchar_t *		srcPath,
	amf_int32 			srcLine,
	const wchar_t * 	message,
	va_list * 			pArglist
)
{
	auto levelString = GetWriterLevelName(level);
	size_t srcPathSize = wcslen(srcPath);

	std::wstring result;
	result.reserve(levelString.length() + srcPathSize + 1024 + 4);

	result += levelString;

	if(srcPathSize)
	{
		result += srcPath;
		result += L" (" + std::to_wstring(srcLine) + L") ";
	}

	result += L" ";

	if(pArglist)
	{
		wchar_t buffer[1024] = {0};
		std::vswprintf(buffer, 1024, message, *pArglist);
		result += buffer;
	}
	else
	{
		result += message;
	}

	return result;
}

bool AMFTraceImpl::CheckLevel(int current, int traceLevel)
{
	if (traceLevel == AMF_TRACE_NOLOG)
		return false;
	return traceLevel <= current;
}

void AMFTraceImpl::Trace(const wchar_t *src_path, amf_int32 line, amf_int32 level, const wchar_t *scope, const wchar_t *message, va_list *pArglist)
{
	if (!CheckLevel(m_level, level))
		return;

	auto resultMessage = FormMessage(level, src_path, line, message, pArglist);

	if (m_consoleEx.enable && CheckLevel(m_consoleEx.level, level))
	{
		if (scope == nullptr)
		{
			m_consoleWriter.Write(scope, message, level);
		}
		else {
			std::map <const wchar_t*, amf_int32> ::iterator it_scope;
			it_scope = m_consoleEx.levels.find(scope);
			if (it_scope == m_consoleEx.levels.end() || CheckLevel(it_scope->second, level))
			{
				m_consoleWriter.Write(scope, message, level);
			}
		}
	}

	std::map<const wchar_t*, AMFTraceWriterEx>::iterator iter;
	for (iter = m_writers.begin(); iter != m_writers.end(); ++iter) {
		if (!iter->second.enable)
			continue;
		if (!CheckLevel(iter->second.level, level))
			continue;

		std::map <const wchar_t *, amf_int32> ::iterator it_scope;
		it_scope = iter->second.levels.find(scope);
		if (it_scope != iter->second.levels.end())
		{
			if (!CheckLevel(it_scope->second, level))
				continue;
		}

		iter->second.pWriter->Write(scope, resultMessage.c_str());
	}
}

void AMFTraceImpl::TraceW(const wchar_t *src_path, amf_int32 line, amf_int32 level, const wchar_t *scope, amf_int32 countArgs, const wchar_t *format, ...)
{
	if (countArgs <= 0)
	{
		Trace(src_path, line, level, scope, format, NULL);
	}
	else
	{
		va_list vl;
		va_start(vl, format);
		Trace(src_path, line, level, scope, format, &vl);
		va_end(vl);
	}
}
