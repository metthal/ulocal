#include <iostream>

#include <ulocal/ulocal.hpp>

using namespace ulocal;

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "client SOCKET_PATH METHOD RESOURCE [[HEADER_NAME HEADER_VALUE] ...] [CONTENT]" << std::endl;
		return 1;
	}

	HttpClient client(argv[1]);
	HttpHeaderTable headers;

	int i = 4;
	for (; i < (argc & ~1); i += 2)
		headers.add_header(argv[i], argv[i + 1]);

	std::string content;
	if ((argc & ~1) != argc)
		content = argv[argc - 1];

	client.send_request(argv[2], argv[3], std::move(content), std::move(headers));
	return 0;
}
