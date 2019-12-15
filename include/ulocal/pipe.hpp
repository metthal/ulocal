#pragma once

#include <memory>
#include <stdexcept>

#include <unistd.h>

#include <ulocal/socket.hpp>

namespace ulocal {

class Pipe
{
public:
	Pipe() : _read_socket(), _write_socket()
	{
		int fds[2];
		if (::pipe(fds) != 0)
			throw std::runtime_error("Unable to create pipe");

		_read_socket = std::make_unique<Socket<NonNetwork>>(fds[0]);
		_write_socket = std::make_unique<Socket<NonNetwork>>(fds[1]);
	}

	Pipe(const Pipe&) = delete;
	Pipe(Pipe&&) noexcept = default;
	~Pipe() = default;

	Pipe& operator=(const Pipe&) = delete;
	Pipe& operator=(Pipe&& o) noexcept
	{
		std::swap(_read_socket, o._read_socket);
		std::swap(_write_socket, o._write_socket);
		return *this;
	}

	int get_read_fd() const { return _read_socket->get_fd(); }
	int get_write_fd() const { return _write_socket->get_fd(); }

	Socket<NonNetwork>* get_read_socket() { return _read_socket.get(); }
	Socket<NonNetwork>* get_write_socket() { return _write_socket.get(); }

private:
	std::unique_ptr<Socket<NonNetwork>> _read_socket;
	std::unique_ptr<Socket<NonNetwork>> _write_socket;
};

} // namespace ulocal
