#include "stdafx.h"

#include "PerformanceCommon.h"

namespace PerformanceTests
{
	clock_t Runner(std::function<void()> func, size_t i_nums)
	{
		clock_t begin = clock();
		for (size_t i = 0; i < i_nums; ++i)
		{
			func();
		}
		return clock() - begin;
	}
}