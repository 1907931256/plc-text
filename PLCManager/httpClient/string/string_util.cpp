#include "string_util.h"

namespace base
{
	template<typename StringType>
	static void StringAppendVT(StringType* dst,
		const typename StringType::value_type* format, va_list ap)
	{
		// 首先尝试用固定大小的小缓冲区.
		typename StringType::value_type stack_buf[1024];

		va_list ap_copy;
		GG_VA_COPY(ap_copy, ap);

		int result = vsnprintfT(stack_buf, arraysize(stack_buf), format, ap_copy);
		va_end(ap_copy);

		if(result>=0 && result<static_cast<int>(arraysize(stack_buf)))
		{
			// 缓冲区合适.
			dst->append(stack_buf, result);
			return;
		}

		// 持续增加缓冲区大小直到合适.
		int mem_length = arraysize(stack_buf);
		while(true)
		{
			if(result < 0)
			{
				// 如果发生错误而不是溢出, 不再继续执行.
				return;
			}
			else
			{
				// 需要"result+1"个字符空间.
				mem_length = result + 1;
			}

			if(mem_length > 32*1024*1024)
			{
				// 已经足够大了, 不再继续尝试. 避免vsnprintfT在大块分配时返回-1而不是
				// 设置溢出.
				return;
			}

			std::vector<typename StringType::value_type> mem_buf(mem_length);

			// 注意: va_list只能用一次. 循环中每次都产生一份新的拷贝而不使用原始的.
			GG_VA_COPY(ap_copy, ap);
			result = vsnprintfT(&mem_buf[0], mem_length, format, ap_copy);
			va_end(ap_copy);

			if((result>=0) && (result<mem_length))
			{
				// 缓冲区合适.
				dst->append(&mem_buf[0], result);
				return;
			}
		}
	}

	void StringAppendV(std::string* dst, const char* format, va_list ap)
	{
		StringAppendVT(dst, format, ap);
	}

	std::string StringPrintf(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		std::string result;
		StringAppendV(&result, format, ap);
		va_end(ap);
		return result;
	}
}