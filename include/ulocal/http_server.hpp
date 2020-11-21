#pragma once

#include <functional>
#include <thread>
#include <unordered_set>

#include <poll.h>

#include <ulocal/http_connection.hpp>
#include <ulocal/http_request.hpp>
#include <ulocal/http_response.hpp>
#include <ulocal/pipe.hpp>
#include <ulocal/route_table.hpp>
#include <ulocal/socket.hpp>
#include <ulocal/version.hpp>

namespace ulocal {

class HttpServer
{
public:
	using RequestCallback = std::function<HttpResponse(const HttpRequest&)>;

	HttpServer(const std::string& local_socket_path)
		: _routes(), _local_socket_path(local_socket_path), _server(), _clients(), _thread(), _control_pipe(), _server_header() {}
	HttpServer(const std::string& local_socket_path, const std::string& server_header) : HttpServer(local_socket_path)
	{
		_server_header = server_header;
	}

	template <typename Fn>
	void endpoint(const std::initializer_list<std::string>& methods, const std::string& route, const Fn& fn)
	{
		_routes.add_route(route, methods, fn);
	}

	bool is_serving() const
	{
		return _server.is_listening();
	}

	void serve()
	{
		_server.listen(_local_socket_path);

		_thread = std::thread([this]() {
			bool running = true;
			while (running)
			{
				std::vector<pollfd> poll_fds;
				poll_fds.reserve(_clients.size() + 1);
				for (const auto& connection : _clients)
					poll_fds.push_back(connection.get_socket().get_poll_fd());
				poll_fds.push_back(_server.get_poll_fd());
				poll_fds.push_back(_control_pipe.get_read_socket()->get_poll_fd());

				auto* server_pollfd = &poll_fds[poll_fds.size() - 2];
				auto* control_pipe_pollfd = &poll_fds[poll_fds.size() - 1];

				auto result = ::poll(poll_fds.data(), poll_fds.size(), -1);
				if (result == -1)
					throw SocketError("Failed while polling HTTP connections");
				else if (result == 0)
					continue;

				if (control_pipe_pollfd->revents & POLLIN)
				{
					_control_pipe.get_read_socket()->read();
					auto command = _control_pipe.get_read_socket()->get_stream().read_until('\0');
					if (command.first == "stop")
						running = false;
				}

				if (server_pollfd->revents & POLLIN)
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
					if (index >= poll_fds.size() - 2)
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
							if (!_routes.has_route(request.get_resource()))
								response = HttpResponse{404};
							else if (!_routes.has_route_for_method(request.get_resource(), request.get_method()))
								response = HttpResponse{405};
							else
							{
								try
								{
									response = _routes.perform_action(request.get_resource(), request.get_method(), request);
								}
								catch (const std::exception& err)
								{
									response = HttpResponse{500};
								}
							}
						}

						if (response)
						{
							response->calculate_content_length();
							if (_server_header)
								response->add_header("Server", _server_header.value());
							response->add_header("Connection", "close");
							response->add_header("X-Framework", "ulocal " ULOCAL_VERSION);

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
		_control_pipe.get_write_socket()->write("stop\0");
	}

private:
	RouteTable<RequestCallback> _routes;
	std::string _local_socket_path;
	Socket<> _server;
	std::vector<HttpConnection> _clients;

	std::thread _thread;
	Pipe _control_pipe;

	std::optional<std::string> _server_header;
};

} // namespace ulocal
