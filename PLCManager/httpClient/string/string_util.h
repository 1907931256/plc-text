#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include<stdafx.h>
#include <windows.h>
#include <vector>
#include "basic_types.h"

#if defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for your compiler in build/build_config.h
#endif

#if defined(COMPILER_GCC)
#define GG_VA_COPY(a, b) (va_copy(a, b))
#elif defined(COMPILER_MSVC)
#define GG_VA_COPY(a, b) (a = b)
#endif


namespace base
{
	inline int snprintf(char* buffer, size_t size, const char* format, ...) {
		va_list arguments;
		va_start(arguments, format);
		int result = vsnprintf(buffer, size, format, arguments);
		va_end(arguments);
		return result;
	}

	inline int strcasecmp(const char* s1, const char* s2) {
		return _stricmp(s1, s2);
	}

	inline int vsnprintf(char* buffer, size_t size,
		const char* format, va_list arguments) {
			int length = _vsprintf_p(buffer, size, format, arguments);
			if (length < 0) {
				if (size > 0)
					buffer[0] = 0;
				return _vscprintf_p(format, arguments);
			}
			return length;
	}

	inline int vsnprintfT(char* buffer, size_t buf_size,
		const char* format, va_list argptr)
	{
		return base::vsnprintf(buffer, buf_size, format, argptr);
	}


	std::string StringPrintf(const char* format, ...);
};
#endif