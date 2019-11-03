#pragma once

#include <sstream>
#include <string>

#include <ulocal/http_header_table.hpp>

namespace ulocal {

class HttpResponse
{
public:
	HttpResponse() : _status_code(200), _content() {}
	HttpResponse(int status_code) : _status_code(status_code), _content() {}
	HttpResponse(const std::string& content) : _status_code(200), _content(content) {}
	HttpResponse(std::string&& content) : _status_code(200), _content(std::move(content)) {}
	HttpResponse(int status_code, const std::string& content) : _status_code(status_code), _content(content) {}
	HttpResponse(int status_code, std::string&& content) : _status_code(status_code), _content(std::move(content)) {}

	int get_status_code() const { return _status_code; }
	const std::string& get_content() const { return _content; }

	template <typename T1, typename T2>
	void add_header(T1&& name, T2&& value)
	{
		_headers.add_header(std::forward<T1>(name), std::forward<T2>(value));
	}

	std::string dump() const
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

		std::string status_name;
		auto itr = status_code_names.find(_status_code);
		if (itr != status_code_names.end())
			status_name = itr->second;
		else
			status_name = "Unknown";

		std::ostringstream ss;
		ss << "HTTP/1.1 " << _status_code << " " << status_name << "\r\n";
		for (const auto* header : _headers)
			ss << header->get_name() << ": " << header->get_value() << "\r\n";
		ss << "\r\n";
		if (!_content.empty())
			ss << _content;
		return ss.str();
	}

private:
	int _status_code;
	HttpHeaderTable _headers;
	std::string _content;
};

} // namespace ulocal
