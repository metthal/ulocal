#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <unordered_set>

#include <poll.h>

#include <ulocal/http_connection.hpp>
#include <ulocal/http_request.hpp>
#include <ulocal/http_response.hpp>
#include <ulocal/route_table.hpp>
#include <ulocal/socket.hpp>

namespace ulocal {

class HttpServer
{
public:
	using RequestCallback = std::function<HttpResponse(const HttpRequest&)>;

	HttpServer(const std::string& local_socket_path)
		: _routes(), _local_socket_path(local_socket_path), _server(), _clients(), _thread(), _terminate(false) {}

	template <typename Fn>
	void endpoint(const std::initializer_list<std::string>& methods, const std::string& route, const Fn& fn)
	{
		_routes.add_route(route, methods, fn);
	}

	void serve()
	{
		_server.listen(_local_socket_path);

		_thread = std::thread([this]() {
			while (!_terminate)
			{
				std::vector<pollfd> poll_fds;
				poll_fds.reserve(_clients.size() + 1);
				for (const auto& connection : _clients)
					poll_fds.push_back(create_pollfd(connection.get_socket()));
				poll_fds.push_back(create_pollfd(_server));

				auto result = ::poll(poll_fds.data(), poll_fds.size(), 1000);
				if (result == -1)
					throw SocketError("Failed while polling HTTP connections");
				else if (result == 0)
					continue;

				if (poll_fds.back().revents & POLLIN)
				{
					auto new_client = _server.accept_connection();
					while (new_client)
					{
						_clients.push_back(std::move(new_client).value());
						new_client = _server.accept_connection();
					}
				}

				std::size_t index = 0;
				for (auto& connection : _clients)
				{
					if (index >= poll_fds.size() - 1)
						break;

					auto& poll_fd = poll_fds[index];
					if (poll_fd.revents & POLLIN)
					{
						std::optional<HttpResponse> response;

						try
						{
							connection.get_socket().read();
						}
						catch (const std::exception& err)
						{
							response = HttpResponse{500};
						}

						auto maybe_request = connection.get_request();
						if (maybe_request)
						{
							auto request = std::move(maybe_request).value();
							auto endpoint = _routes.has_route(request.get_resource());
							if (!endpoint)
								response = HttpResponse{404};
							else if (!endpoint->supports_method(request.get_method()))
								response = HttpResponse{405};
							else
							{
								try
								{
									response = endpoint->perform(request);
								}
								catch (const std::exception& err)
								{
									response = HttpResponse{500};
								}
							}
						}

						if (response)
						{
							response->add_header("Connection", "close");
							try
							{
								connection.get_socket().write(response->dump());
							}
							catch (const std::exception& err)
							{
								;
							}
							connection.get_socket().close();
						}
					}

					if (poll_fd.revents & POLLHUP)
						connection.get_socket().close();

					++index;
				}

				auto remove_itr = std::remove_if(_clients.begin(), _clients.end(), [](const auto& connection) {
					return connection.get_socket().is_closed();
				});
				_clients.erase(remove_itr, _clients.end());
			}
		});
	}

	void wait_until_done()
	{
		_thread.join();
	}

	void terminate()
	{
		_terminate = true;
	}

private:
	pollfd create_pollfd(const Socket& socket)
	{
		return {socket.get_fd(), POLLIN, 0};
	}

	RouteTable<RequestCallback> _routes;
	std::string _local_socket_path;
	Socket _server;
	std::vector<HttpConnection> _clients;

	std::thread _thread;
	std::atomic_bool _terminate;
};

} // namespace ulocal
