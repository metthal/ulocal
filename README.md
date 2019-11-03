# ulocal

ulocal is small header-only C++17 library without any third party dependencies providing HTTP API over unix domain sockets.

Usual way to manage and communicate with daemon applications is through IPC (inter-process communication) mechanism from userspace through some interface.
Quite often in form of unix domain sockets to which you connect (through some application) and you can communicate with the daemon.
There are frameworks like dbus which standardize this process but integrating them is often not that easy without incorporating loads of third-party libraries
into your application or studying the specifications. This library tries to solve it by providing very lightweight library which does the same
It does not have a lot of advanced features but it is not the purpose of this library to replace dbus or other solutions.

Choosing HTTP as a protocol for communication was done because there is HTTP client implementation basically in every single language out there.
This allows developers and communities around their projects to build userspace tools however they want.
