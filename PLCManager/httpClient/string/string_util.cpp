#include "string_util.h"

namespace base
{
	template<typename StringType>
	static void StringAppendVT(StringType* dst,
		const typename StringType::value_type* format, va_list ap)
	{
		// ���ȳ����ù̶���С��С������.
		typename StringType::value_type stack_buf[1024];

		va_list ap_copy;
		GG_VA_COPY(ap_copy, ap);

		int result = vsnprintfT(stack_buf, arraysize(stack_buf), format, ap_copy);
		va_end(ap_copy);

		if(result>=0 && result<static_cast<int>(arraysize(stack_buf)))
		{
			// ����������.
			dst->append(stack_buf, result);
			return;
		}

		// �������ӻ�������Сֱ������.
		int mem_length = arraysize(stack_buf);
		while(true)
		{
			if(result < 0)
			{
				// �������������������, ���ټ���ִ��.
				return;
			}
			else
			{
				// ��Ҫ"result+1"���ַ��ռ�.
				mem_length = result + 1;
			}

			if(mem_length > 32*1024*1024)
			{
				// �Ѿ��㹻����, ���ټ�������. ����vsnprintfT�ڴ�����ʱ����-1������
				// �������.
				return;
			}

			std::vector<typename StringType::value_type> mem_buf(mem_length);

			// ע��: va_listֻ����һ��. ѭ����ÿ�ζ�����һ���µĿ�������ʹ��ԭʼ��.
			GG_VA_COPY(ap_copy, ap);
			result = vsnprintfT(&mem_buf[0], mem_length, format, ap_copy);
			va_end(ap_copy);

			if((result>=0) && (result<mem_length))
			{
				// ����������.
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