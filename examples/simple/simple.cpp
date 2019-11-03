#include <iostream>

#include <ulocal/http_server.hpp>

using namespace ulocal;

int main()
{
	HttpServer server("simple.sock");
	server.endpoint({ "GET", "POST" }, "/", [](const HttpRequest& request) -> HttpResponse {
		std::cout << "Method: " << request.get_method() << std::endl;
		std::cout << "Resource: " << request.get_resource() << std::endl;
		std::cout << "Content: " << request.get_content() << std::endl;
		return 200;
	});
	server.serve();

	std::cin.get();
	server.terminate();
	server.wait_until_done();
}
