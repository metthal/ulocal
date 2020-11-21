#pragma once

#include <unordered_map>

#include <ulocal/endpoint.hpp>
#include <ulocal/utils.hpp>

namespace ulocal {

template <typename Callback>
class RouteTable
{
public:
	RouteTable() : _table() {}

	bool has_route(const std::string& route) const
	{
		return _table.find(route) != _table.end();
	}

	bool has_route_for_method(const std::string& route, const std::string& method) const
	{
		auto route_itr = _table.find(route);
		if (route_itr == _table.end())
			return false;

		return route_itr->second.find(method) != route_itr->second.end();
	}

	template <typename R, typename M, typename C>
	void add_route(R&& route, M&& methods, C&& callback)
	{
		auto itr = _table.find(route);
		if (itr == _table.end())
			std::tie(itr, std::ignore) = _table.emplace(route, MethodTable{});

		auto& method_table = itr->second;
		for (const auto& method : methods)
		{
			auto method_itr = method_table.find(method);
			if (method_itr == method_table.end())
				method_table.emplace(method, callback);
			else
				method_itr->second = callback;
		}
	}

	template <typename... Args>
	auto perform_action(const std::string& route, const std::string& method, Args&&... args) const
	{
		return _table.at(route).at(method)(std::forward<Args>(args)...);
	}

private:
	using MethodTable = std::unordered_map<std::string, Callback, CaseInsensitiveHash, CaseInsensitiveCompare>;

	std::unordered_map<std::string, MethodTable, CaseInsensitiveHash, CaseInsensitiveCompare> _table;
};

} // namespace ulocal
