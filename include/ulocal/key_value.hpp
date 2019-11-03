#pragma once

#include <string>

#include <ulocal/utils.hpp>

namespace ulocal {

namespace detail {

template <typename... Args>
struct ValueGetter {};

template <typename T>
struct ValueGetter<T>
{
	using allow = std::enable_if_t<std::is_integral_v<T>, void>;

	T operator()(const std::string& str) const
	{
		return std::stol(str);
	}
};

template <>
struct ValueGetter<bool>
{
	bool operator()(const std::string& str) const
	{
		return icase_compare(str, "true");
	}
};

}

class KeyValue
{
public:
	template <typename T1, typename T2>
	KeyValue(T1&& name, T2&& value) : _name(std::forward<T1>(name)), _value(std::forward<T2>(value)) {}

	const std::string& get_name() const { return _name; }
	const std::string& get_value() const { return _value; }

	template <typename T>
	T get_value_as() const
	{
		return detail::ValueGetter<T>{}(_value);
	}

private:
	template <typename T>
	T get_value_as(int)
	{
		return std::stol(_value);
	}

	std::string _name, _value;
};

} // namespace ulocal
