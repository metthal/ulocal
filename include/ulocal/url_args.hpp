#pragma once

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

private:
	std::vector<const UrlArg*> _args;
	std::unordered_map<std::string, UrlArg> _table;
};

} // namespace ulocal
