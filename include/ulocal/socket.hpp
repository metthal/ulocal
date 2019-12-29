#pragma once

#include <string>

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <ulocal/string_stream.hpp>

namespace ulocal {

struct Network
{
	static ssize_t read(int fd, void* buf, size_t len)
	{
		return ::recv(fd, buf, len, 0);
	}

	static ssize_t write(int fd, const void* buf, size_t len)
	{
		return ::send(fd, buf, len, 0);
	}
};

struct NonNetwork
{
	static ssize_t read(int fd, void* buf, size_t len)
	{
		return ::read(fd, buf, len);
	}

	static ssize_t write(int fd, const void* buf, size_t len)
	{
		return ::write(fd, buf, len);
	}
};

class SocketError : public std::exception
{
public:
	SocketError(const char* msg) noexcept : _msg(msg) {}

	virtual const char* what() const noexcept { return _msg; }

private:
	const char* _msg;
};

template <typename SocketOp = Network>
class Socket
{
public:
	Socket() : Socket(::socket(AF_UNIX, SOCK_STREAM, 0)) {}

	Socket(int fd) : _fd(fd), _stream(4096)
	{
		if (_fd < 0)
			throw SocketError("Unable to create socket");

		if (::fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
			throw SocketError("Unable to switch socket to non-blocking mode");
	}

	~Socket()
	{
		close();
	}

	Socket(Socket&& rhs) noexcept : _fd(rhs._fd), _stream(std::move(rhs._stream))
	{
		rhs._fd = 0;
	}

	Socket& operator=(Socket&& rhs) noexcept
	{
		_fd = rhs._fd;
		_stream = std::move(rhs._stream);
		rhs._fd = 0;
		return *this;
	}

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	int get_fd() const { return _fd; }
	StringStream& get_stream() { return _stream; }
	pollfd get_poll_fd() const { return {_fd, POLLIN, 0}; }

	bool is_closed() const { return _fd == 0; }

	void connect(const std::string& file_path)
	{
		auto sa = create_sockaddr(file_path);
		if (::connect(_fd, reinterpret_cast<sockaddr*>(&sa), SUN_LEN(&sa)) < 0)
			throw SocketError("Error while connecting to the local socket");
	}

	void listen(const std::string& file_path)
	{
		auto sa = create_sockaddr(file_path);
		if (::bind(_fd, reinterpret_cast<sockaddr*>(&sa), SUN_LEN(&sa)) < 0)
			throw SocketError("Unable to bind the local socket");

		if (::listen(_fd, 16) < 0)
			throw SocketError("Unable to start listening to the local socket");
	}

	std::optional<Socket> accept_connection()
	{
		auto client_fd = ::accept(_fd, nullptr, nullptr);
		if (client_fd < 0)
		{
			if (errno == EWOULDBLOCK)
				return std::nullopt;

			throw SocketError("Error while accepting new connection on the local socket");
		}

		return client_fd;
	}

	void read()
	{
		int n = 0;

		do
		{
			n = SocketOp::read(_fd, _stream.get_writable_buffer(), _stream.get_writable_size());
			if (n < 0)
			{
				if (errno == EWOULDBLOCK)
					return;

				throw SocketError("Error while reading data from the local socket");
			}

			_stream.increase_used(n);
		}
		while (n > 0);
	}

	void write(const std::string& str)
	{
		std::size_t sent = 0;
		while (sent < str.length())
		{
			auto n = SocketOp::write(_fd, str.data() + sent, str.length() - sent);
			if (n < 0)
			{
				if (errno == EWOULDBLOCK)
					return;

				throw SocketError("Error while writing data to the local socket");
			}

			sent += static_cast<std::size_t>(n);
		}
	}

	void close()
	{
		if (_fd != 0)
		{
			::shutdown(_fd, SHUT_RDWR);
			::close(_fd);
			_fd = 0;
		}
	}

private:
	sockaddr_un create_sockaddr(const std::string& file_path)
	{
		sockaddr_un sa;
		memset(&sa, 0, sizeof(sockaddr_un));

		sa.sun_family = AF_UNIX;
		std::strncpy(sa.sun_path, file_path.c_str(), std::min(file_path.length(), sizeof(sa.sun_path) - 1));

		return sa;
	}

	int _fd;
	StringStream _stream;
};

} // namespace ulocal
