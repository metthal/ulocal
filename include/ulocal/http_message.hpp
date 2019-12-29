#pragma once

#include <ulocal/http_header_table.hpp>

namespace ulocal {

class HttpMessage
{
public:
	HttpMessage() : _content() {}
	HttpMessage(const std::string&& content) : _content(content), _headers() {}
	HttpMessage(std::string&& content) : _content(std::move(content)), _headers() {}

	template <typename Content, typename Headers>
	HttpMessage(Content&& content, Headers&& headers) : _content(std::forward<Content>(content)), _headers(std::forward<Headers>(headers)) {}

	HttpMessage(const HttpMessage&) = default;
	HttpMessage(HttpMessage&&) noexcept = default;
	virtual ~HttpMessage() = default;

	HttpMessage& operator=(const HttpMessage&) = default;
	HttpMessage& operator=(HttpMessage&&) noexcept = default;

	const std::string& get_content() const { return _content; }
	const HttpHeaderTable& get_headers() const { return _headers; }
	const HttpHeader* get_header(const std::string& name) const { return _headers.get_header(name); }

	bool has_header(const std::string& name) const { return _headers.has_header(name); }

	template <typename Name, typename Value>
	void add_header(Name&& name, Value&& value)
	{
		_headers.add_header(std::forward<Name>(name), std::forward<Value>(value));
	}

	void calculate_content_length()
	{
		if (auto content_length = _headers.get_header("Content-Length"); !content_length && !_content.empty())
			_headers.add_header("Content-Length", _content.length());
	}

	virtual std::string dump() const = 0;

protected:
	std::string _content;
	HttpHeaderTable _headers;
};

} // namespace ulocal
