#pragma once

#include <sstream>
#include <string>

#include <ulocal/http_message.hpp>
#include <ulocal/url_args.hpp>

namespace ulocal {

class HttpRequest : public HttpMessage
{
public:
	template <typename Method, typename Resource>
	HttpRequest(Method&& method, Resource&& resource)
		: HttpRequest(std::forward<Method>(method), std::forward<Resource>(resource), std::string{}) {}

	template <typename Method, typename Resource, typename Content>
	HttpRequest(Method&& method, Resource&& resource, Content&& content)
		: HttpRequest(std::forward<Method>(method), std::forward<Resource>(resource), HttpHeaderTable{}, std::forward<Content>(content)) {}

	template <typename Method, typename Resource, typename Headers, typename Content>
	HttpRequest(Method&& method, Resource&& resource, Headers&& headers, Content&& content)
		: HttpRequest(std::forward<Method>(method), std::forward<Resource>(resource), UrlArgs{}, std::forward<Headers>(headers), std::forward<Content>(content))
	{
		auto [new_resource, args] = UrlArgs::parse_from_resource(_resource);
		_resource = std::move(new_resource);
		_args = std::move(args);
	}

	template <typename Method, typename Resource, typename Args, typename Headers, typename Content>
	HttpRequest(Method&& method, Resource&& resource, Args&& args, Headers&& headers, Content&& content)
		: HttpMessage(std::forward<Content>(content), std::forward<Headers>(headers))
		, _method(std::forward<Method>(method))
		, _resource(std::forward<Resource>(resource))
		, _args(std::forward<Args>(args))
	{
	}

	HttpRequest(const HttpRequest&) = default;
	HttpRequest(HttpRequest&&) noexcept = default;
	virtual ~HttpRequest() = default;

	HttpRequest& operator=(const HttpRequest&) = default;
	HttpRequest& operator=(HttpRequest&&) noexcept = default;

	const std::string& get_method() const { return _method; }
	const std::string& get_resource() const { return _resource; }
	const UrlArg* get_argument(const std::string& name) const { return _args.get_arg(name); }
	const UrlArgs& get_arguments() const { return _args; }

	bool has_arg(const std::string& name) const { return _args.has_arg(name); }

	virtual std::string dump() const override
	{
		std::ostringstream ss;
		ss << _method << ' ' << _resource << _args << " HTTP/1.1\r\n";
		for (const auto* header : _headers)
			ss << header->get_name() << ": " << header->get_value() << "\r\n";
		ss << "\r\n";
		if (!_content.empty())
			ss << _content;
		return ss.str();
	}

private:
	std::string _method;
	std::string _resource;
	UrlArgs _args;
};

} // namespace ulocal
