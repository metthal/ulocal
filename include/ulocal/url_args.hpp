#pragma once

#include <sstream>
#include <unordered_map>
#include <vector>

#include <ulocal/url_arg.hpp>

namespace ulocal {

class UrlArgs
{
public:
	UrlArgs() : _args(), _table() {}

	auto begin() const { return _args.begin(); }
	auto end() const { return _args.end(); }
	std::size_t size() const { return _args.size(); }

	void clear()
	{
		_args.clear();
		_table.clear();
	}

	bool has_arg(const std::string& name) const
	{
		return _table.find(name) != _table.end();
	}

	const UrlArg* get_arg(const std::string& name) const
	{
		auto itr = _table.find(name);
		if (itr == _table.end())
			return nullptr;

		return &itr->second;
	}

	template <typename T1, typename T2>
	void add_arg(T1&& name, T2&& value)
	{
		auto itr = _table.find(name);
		if (itr == _table.end())
		{
			auto header = UrlArg{std::forward<T1>(name), std::forward<T2>(value)};
			std::tie(itr, std::ignore) = _table.emplace(header.get_name(), std::move(header));
			_args.push_back(&itr->second);
		}
	}

	static std::pair<std::string, UrlArgs> parse_from_resource(std::string_view resource)
	{
		UrlArgs result;

		auto url_params_start = resource.find('?');
		if (url_params_start != std::string::npos)
		{
			auto url_args = std::string_view{resource.data() + url_params_start + 1, resource.length() - url_params_start - 1};
			auto pos  = url_args.find('&');
			decltype(pos) old_pos = 0;
			while (pos != std::string::npos)
			{
				auto arg = std::string_view{url_args.data() + old_pos, pos - old_pos};
				auto [key, value] = parse_url_arg(arg);
				result.add_arg(std::move(key), std::move(value));
				old_pos = pos + 1;
				pos = url_args.find('&', old_pos);
			}

			auto last_arg = std::string_view{url_args.data() + old_pos, url_args.length() - old_pos};
			auto [key, value] = parse_url_arg(last_arg);
			result.add_arg(std::move(key), std::move(value));
		}
		else
			url_params_start = resource.length();

		return {std::string{resource.data(), url_params_start}, result};
	}

	friend std::ostream& operator<<(std::ostream& out, const UrlArgs& args)
	{
		if (args._args.empty())
			return out;

		for (std::size_t i = 0; i < args._args.size(); ++i)
		{
			out << (i == 0 ? '?' : '&')
				<< url_encode(args._args[i]->get_name())
				<< '='
				<< url_encode(args._args[i]->get_value());
		}

		return out;
	}

private:
	static std::pair<std::string, std::string> parse_url_arg(const std::string_view& arg)
	{
		auto value_pos = arg.find('=');
		if (value_pos == std::string::npos)
			return { std::string{arg}, std::string{} };

		return {
			url_decode(std::string_view{arg.data(), value_pos}),
			url_decode(std::string_view{arg.data() + value_pos + 1, arg.length() - value_pos - 1})
		};
	}

	std::vector<const UrlArg*> _args;
	std::unordered_map<std::string, UrlArg> _table;
};

} // namespace ulocal
