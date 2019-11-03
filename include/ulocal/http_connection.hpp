#pragma once

#include <ulocal/http_request_parser.hpp>
#include <ulocal/socket.hpp>

namespace ulocal {

class HttpConnection
{
public:
	HttpConnection(Socket&& socket) : _socket(std::move(socket)), _request_parser() {}
	HttpConnection(const HttpConnection&) = delete;
	HttpConnection(HttpConnection&&) noexcept = default;

	HttpConnection& operator=(const HttpConnection&) = delete;
	HttpConnection& operator=(HttpConnection&&) noexcept = default;

	Socket& get_socket() { return _socket; }
	const Socket& get_socket() const { return _socket; }
	std::optional<HttpRequest> get_request() { return _request_parser.parse(_socket.get_stream()); }

private:
	Socket _socket;
	HttpRequestParser _request_parser;
};

} // namespace ulocal
