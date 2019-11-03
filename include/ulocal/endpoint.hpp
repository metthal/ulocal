#pragma once

#include <string>
#include <unordered_set>

#include <ulocal/utils.hpp>

namespace ulocal {

template <typename Callback>
class Endpoint
{
public:
	template <typename R, typename M, typename C>
	Endpoint(R&& route, M&& methods, C&& callback)
		: _route(std::forward<R>(route)), _methods(std::forward<M>(methods)), _callback(std::forward<C>(callback)) {}

	const std::string& get_route() const { return _route; }

	bool supports_method(const std::string& method) const
	{
		return _methods.find(method) != _methods.end();
	}

	template <typename... Args>
	auto perform(Args&&... args) const
	{
		return _callback(std::forward<Args>(args)...);
	}

private:
	std::string _route;
	std::unordered_set<std::string, CaseInsensitiveHash, CaseInsensitiveCompare> _methods;
	Callback _callback;
};

} // namespace ulocal
