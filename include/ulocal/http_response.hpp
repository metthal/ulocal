#pragma once

#include <sstream>
#include <string>

#include <ulocal/http_message.hpp>

namespace ulocal {

class HttpResponse : public HttpMessage
{
public:
	HttpResponse() : HttpResponse(200) {}
	HttpResponse(int status_code) : HttpResponse(status_code, std::string{}) {}
	HttpResponse(const std::string& content) : HttpResponse(200, content) {}
	HttpResponse(std::string&& content) : HttpResponse(200, std::move(content)) {}

	template <typename Content>
	HttpResponse(int status_code, Content&& content) : HttpResponse(status_code, std::optional<std::string>{}, HttpHeaderTable{}, std::forward<Content>(content)) {}

	template <typename Reason, typename Headers, typename Content>
	HttpResponse(int status_code, Reason&& reason, Headers&& headers, Content&& content)
		: HttpMessage(std::forward<Content>(content), std::forward<Headers>(headers))
		, _status_code(status_code)
		, _reason(std::forward<Reason>(reason))
	{
	}

	HttpResponse(const HttpResponse&) = default;
	HttpResponse(HttpResponse&&) noexcept = default;
	virtual ~HttpResponse() = default;

	HttpResponse& operator=(const HttpResponse&) = default;
	HttpResponse& operator=(HttpResponse&&) noexcept = default;

	int get_status_code() const { return _status_code; }

	std::string get_reason() const
	{
		static const std::unordered_map<int, std::string_view> status_code_names = {
			{ 100, "Continue" },
			{ 101, "Switching Protocols" },
			{ 200, "OK" },
			{ 201, "Created" },
			{ 202, "Accepted" },
			{ 203, "Non-Authoritative Information" },
			{ 204, "No Content" },
			{ 205, "Reset Content" },
			{ 206, "Partial Content" },
			{ 300, "Multiple Choices" },
			{ 301, "Moved Permanently" },
			{ 302, "Found" },
			{ 303, "See Other" },
			{ 304, "Not Modified" },
			{ 305, "Use Proxy" },
			{ 307, "Temporary Redirect" },
			{ 400, "Bad Request" },
			{ 401, "Unauthorized" },
			{ 402, "Payment Required" },
			{ 403, "Forbidden" },
			{ 404, "Not Found" },
			{ 405, "Method Not Allowed" },
			{ 406, "Not Acceptable" },
			{ 407, "Proxy Authentication Required" },
			{ 408, "Request Timeout" },
			{ 409, "Conflict" },
			{ 410, "Gone" },
			{ 411, "Length Required" },
			{ 412, "Precondition Failed" },
			{ 413, "Request Entity Too Large" },
			{ 414, "Request-URI Too Long" },
			{ 415, "Unsupported Media Type" },
			{ 416, "Requested Range Not Satisfiable" },
			{ 417, "Expectation Failed" },
			{ 500, "Internal Server Error" },
			{ 501, "Not Implemented" },
			{ 502, "Bad Gateway" },
			{ 503, "Service Unavailable" },
			{ 504, "Gateway Timeout" },
			{ 505, "HTTP Version Not Supported" }
		};

		if (_reason.has_value())
			return _reason.value();
		else
		{
			auto itr = status_code_names.find(_status_code);
			if (itr != status_code_names.end())
				return std::string{itr->second};
		}

		return "Unknown";
	}

	virtual std::string dump() const override
	{
		std::ostringstream ss;
		ss << "HTTP/1.1 " << _status_code << " " << get_reason() << "\r\n";
		for (const auto* header : _headers)
			ss << header->get_name() << ": " << header->get_value() << "\r\n";
		ss << "\r\n";
		if (!_content.empty())
			ss << _content;
		return ss.str();
	}

private:
	int _status_code;
	std::optional<std::string> _reason;
};

} // namespace ulocal
