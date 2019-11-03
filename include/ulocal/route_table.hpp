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

	const Endpoint<Callback>* has_route(const std::string& route)
	{
		auto itr = _table.find(route);
		if (itr == _table.end())
			return nullptr;

		return &itr->second;
	}

	template <typename R, typename M, typename C>
	void add_route(R&& route, M&& methods, C&& callback)
	{
		auto itr = _table.find(route);
		if (itr == _table.end())
			_table.emplace(route, Endpoint<Callback>{route, std::forward<M>(methods), std::forward<C>(callback)});
	}

private:
	std::unordered_map<std::string, Endpoint<Callback>, CaseInsensitiveHash, CaseInsensitiveCompare> _table;
};

} // namespace ulocal
