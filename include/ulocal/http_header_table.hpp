#pragma once

#include <cstdint>
#include <unordered_map>

#include <ulocal/http_header.hpp>

namespace ulocal {

class HttpHeaderTable
{
public:
	HttpHeaderTable() : _headers(), _table() {}

	auto begin() const { return _headers.begin(); }
	auto end() const { return _headers.end(); }
	std::size_t size() const { return _headers.size(); }

	void clear()
	{
		_headers.clear();
		_table.clear();
	}

	bool has_header(const std::string& name) const
	{
		return _table.find(name) != _table.end();
	}

	HttpHeader* get_header(const std::string& name)
	{
		auto itr = _table.find(name);
		if (itr == _table.end())
			return nullptr;

		return &itr->second;
	}

	const HttpHeader* get_header(const std::string& name) const
	{
		auto itr = _table.find(name);
		if (itr == _table.end())
			return nullptr;

		return &itr->second;
	}

	template <typename T1, typename T2>
	void add_header(T1&& name, T2&& value)
	{
		auto itr = _table.find(name);
		if (itr == _table.end())
		{
			auto header = HttpHeader{std::forward<T1>(name), std::forward<T2>(value)};
			std::tie(itr, std::ignore) = _table.emplace(header.get_name(), std::move(header));
			_headers.push_back(&itr->second);
		}
	}

private:
	std::vector<const HttpHeader*> _headers;
	std::unordered_map<std::string, HttpHeader, CaseInsensitiveHash, CaseInsensitiveCompare> _table;
};

} // namespace ulocal
