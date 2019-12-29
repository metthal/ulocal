#pragma once

#include <string>

#include <poll.h>

#include <ulocal/http_header_table.hpp>
#include <ulocal/http_request.hpp>
#include <ulocal/http_response.hpp>
#include <ulocal/http_response_parser.hpp>
#include <ulocal/socket.hpp>

namespace ulocal {

class RequestError : public std::exception
{
public:
	RequestError(const char* msg) noexcept : _msg(msg) {}

	virtual const char* what() const noexcept { return _msg; }

private:
	const char* _msg;
};

class HttpClient
{
public:
	HttpClient(const std::string& local_socket_path) : _local_socket_path(local_socket_path) {}

	template <typename Method, typename Resource>
	HttpResponse send_request(Method&& method, Resource&& resource)
	{
		return send_request(
			std::forward<Method>(method),
			std::forward<Resource>(resource),
			std::string{},
			HttpHeaderTable{}
		);
	}

	template <typename Method, typename Resource, typename Content>
	HttpResponse send_request(Method&& method, Resource&& resource, Content&& content)
	{
		return send_request(
			std::forward<Method>(method),
			std::forward<Resource>(resource),
			std::forward<Content>(content),
			HttpHeaderTable{}
		);
	}

	template <typename Method, typename Resource, typename Content, typename Headers>
	HttpResponse send_request(Method&& method, Resource&& resource, Content&& content, Headers&& headers)
	{
		HttpRequest request{
			std::forward<Method>(method),
			std::forward<Resource>(resource),
			std::forward<Headers>(headers),
			std::forward<Content>(content)
		};
		request.calculate_content_length();

		Socket<> socket;
		socket.connect(_local_socket_path);
		socket.write(request.dump());

		HttpResponseParser response_parser;
		std::optional<HttpResponse> maybe_response;
		auto pollfd = socket.get_poll_fd();
		bool still_poll = true;

		while (still_poll && !maybe_response)
		{
			auto result = ::poll(&pollfd, 1, -1);
			if (result == -1)
				throw RequestError("Unable to obtain response from the server");

			if (pollfd.revents & POLLIN)
			{
				socket.read();
				maybe_response = response_parser.parse(socket.get_stream());
			}

			if (pollfd.revents & POLLHUP)
				still_poll = false;
		}

		if (!maybe_response)
			throw RequestError("Server closed connection unexpectedly");

		return std::move(maybe_response).value();
	}

private:
	std::string _local_socket_path;
};


} // namespace ulocal
