#pragma once


namespace PerformanceTests
{

	clock_t Runner(std::function<void()> func, size_t i_nums);
	
	/*template <typename Func, typename Args... args>
	clock_t Runner(Func i_func, size_t i_nums, Args... i_args)
	{
		clock_t begin = clock();
		for (size_t i = 0; i < i_nums; ++i)
		{
			i_func(std::forward(i_args)...);
		}
		return clock() - i_func;
	}*/

} // PerformanceTests