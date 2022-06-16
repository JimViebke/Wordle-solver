#pragma once

#include <vector>
#include <chrono>

size_t random_int_from(const size_t min, const size_t max)
{
	return (rand() % (max - min + 1)) + min;
}

template<class C, class T>
auto contains(const C& container, const T& element)
-> decltype(end(container), true)
{
	return std::find(begin(container), end(container), element) != end(container);
}

auto current_time_in_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

auto current_time_in_us()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
