#include <signal.h>

#include <ulocal/ulocal.hpp>

#include "json.hpp"

using namespace ulocal;
using json = nlohmann::json;

sig_atomic_t terminate = 0;

void handle_terminate(int)
{
	terminate = 1;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
		return 1;

	signal(SIGINT, &handle_terminate);
	signal(SIGTERM, &handle_terminate);

	auto ok_handler = [](const HttpRequest& request) -> HttpResponse {
		auto response = json{
			{"endpoint", "/"},
			{"request", {
				{"method", request.get_method()},
				{"resource", request.get_resource()},
				{"headers", json::object()},
				{"args", json::object()},
				{"content", request.get_content()}
			}}
		};

		for (const auto& header : request.get_headers())
			response["request"]["headers"][header->get_name()] = header->get_value();

		for (const auto& arg : request.get_arguments())
			response["request"]["args"][arg->get_name()] = arg->get_value();

		return {200, response.dump()};
	};

	HttpServer server(argv[1]);
	server.endpoint({"GET", "POST", "PUT", "DELETE"}, "/", ok_handler);
	server.endpoint({"GET"}, "/get", ok_handler);
	server.endpoint({"POST"}, "/post", ok_handler);
	server.endpoint({"GET"}, "/error/500", [&](const HttpRequest&) -> HttpResponse {
		return 500;
	});
	server.endpoint({"GET"}, "/different_handlers_for_different_methods", ok_handler);
	server.endpoint({"POST"}, "/different_handlers_for_different_methods", [&](const HttpRequest&) -> HttpResponse {
		return 500;
	});

	server.serve();

	while (!terminate)
	{
		using namespace std::literals;
		std::this_thread::sleep_for(1s);
	}

	server.terminate();
	server.wait_until_done();
}
