#pragma once

#include <string>

#include <ulocal/http_header_table.hpp>
#include <ulocal/url_args.hpp>

namespace ulocal {

class HttpRequest
{
public:
	template <typename T1, typename T2, typename T3>
	HttpRequest(T1&& method, T2&& resource, UrlArgs&& args, HttpHeaderTable&& headers, T3&& content)
		: _method(std::forward<T1>(method)), _resource(std::forward<T2>(resource)), _args(std::move(args)),
		_headers(std::move(headers)), _content(std::forward<T3>(content)) {}

	const std::string& get_method() const { return _method; }
	const std::string& get_resource() const { return _resource; }
	const UrlArg* get_argument(const std::string& name) const { return _args.get_arg(name); }
	const UrlArgs& get_arguments() const { return _args; }
	const HttpHeader* get_header(const std::string& name) const { return _headers.get_header(name); }
	const HttpHeaderTable& get_headers() const { return _headers; }
	const std::string& get_content() const { return _content; }

	bool has_arg(const std::string& name) const { return _args.has_arg(name); }
	bool has_header(const std::string& name) const { return _headers.has_header(name); }

	template <typename T1, typename T2>
	void add_header(T1&& name, T2&& value)
	{
		_headers.add_header(std::forward<T1>(name), std::forward<T2>(value));
	}

private:
	std::string _method;
	std::string _resource;
	UrlArgs _args;
	HttpHeaderTable _headers;
	std::string _content;
};

} // namespace ulocal
