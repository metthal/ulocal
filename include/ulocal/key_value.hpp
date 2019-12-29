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

	static T convert(const std::string& str) { return std::stol(str); }
};

template <>
struct ValueGetter<bool>
{
	static bool convert(const std::string& str) { return icase_compare(str, "true"); }
};

template <typename... Args>
struct ValueSetter {};

template <>
struct ValueSetter<char*>
{
	static std::string convert(char* value) { return value; }
};

template <>
struct ValueSetter<const char*>
{
	static std::string convert(const char* value) { return value; }
};

template <>
struct ValueSetter<std::string>
{
	static std::string convert(const std::string& value) { return value; }
};

template <>
struct ValueSetter<bool>
{
	static std::string convert(bool value) { return value ? "true" : "false"; }
};

template <typename T>
struct ValueSetter<T>
{
	using allow = std::enable_if_t<std::is_integral_v<T>, bool>;

	static std::string convert(T value) { return std::to_string(value); }
};

}

class KeyValue
{
public:
	template <typename Name, typename Value>
	KeyValue(Name&& name, Value&& value) : _name(std::forward<Name>(name)), _value(detail::ValueSetter<std::decay_t<Value>>::convert(value)) {}

	const std::string& get_name() const { return _name; }
	const std::string& get_value() const { return _value; }

	template <typename T>
	T get_value_as() const
	{
		return detail::ValueGetter<T>::convert(_value);
	}

	template <typename Value>
	void set_value(const Value& value)
	{
		_value = detail::ValueSetter<std::decay_t<Value>>::convert(value);
	}

private:
	std::string _name, _value;
};

} // namespace ulocal
