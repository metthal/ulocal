#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ulocal/http_request_parser.hpp>

using namespace ::testing;
using namespace ulocal;

class TestHttpRequestParser : public ::testing::Test {};

TEST_F(TestHttpRequestParser,
ParseSimpleRequest) {
	StringStream stream(
		"GET / HTTP/1.1\r\n"
		"\r\n"
	);

	HttpRequestParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "GET");
	EXPECT_EQ(request.get_resource(), "/");
	EXPECT_EQ(request.get_arguments().size(), 0u);
	EXPECT_EQ(request.get_headers().size(), 0u);
	EXPECT_EQ(request.get_content(), std::string{});
}

TEST_F(TestHttpRequestParser,
ParseRequestWithHeaders) {
	StringStream stream(
		"GET / HTTP/1.1\r\n"
		"Accept: application/json\r\n"
		"Connection: close\r\n"
		"\r\n"
	);

	HttpRequestParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "GET");
	EXPECT_EQ(request.get_resource(), "/");
	EXPECT_EQ(request.get_arguments().size(), 0u);
	EXPECT_EQ(request.get_headers().size(), 2u);
	EXPECT_EQ(request.get_header("accept")->get_value(), "application/json");
	EXPECT_EQ(request.get_header("connection")->get_value(), "close");
	EXPECT_EQ(request.get_content(), std::string{});
}

TEST_F(TestHttpRequestParser,
ParseRequestWithContent) {
	StringStream stream(
		"GET / HTTP/1.1\r\n"
		"Accept: application/json\r\n"
		"Connection: close\r\n"
		"Content-Length: 12\r\n"
		"\r\n"
		"Hello World!"
	);

	HttpRequestParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "GET");
	EXPECT_EQ(request.get_resource(), "/");
	EXPECT_EQ(request.get_arguments().size(), 0u);
	EXPECT_EQ(request.get_headers().size(), 3u);
	EXPECT_EQ(request.get_header("accept")->get_value(), "application/json");
	EXPECT_EQ(request.get_header("connection")->get_value(), "close");
	EXPECT_EQ(request.get_header("content-length")->get_value_as<std::uint64_t>(), 12u);
	EXPECT_EQ(request.get_content(), "Hello World!");
}

TEST_F(TestHttpRequestParser,
ParseUrlWithQueryArguments) {
	StringStream stream(
		"GET /endpoint?arg1=value1&arg2=value2 HTTP/1.1\r\n"
		"Accept: application/json\r\n"
		"Connection: close\r\n"
		"\r\n"
	);

	HttpRequestParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "GET");
	EXPECT_EQ(request.get_resource(), "/endpoint");
	EXPECT_EQ(request.get_arguments().size(), 2u);
	EXPECT_EQ(request.get_argument("arg1")->get_value(), "value1");
	EXPECT_EQ(request.get_argument("arg2")->get_value(), "value2");
	EXPECT_EQ(request.get_headers().size(), 2u);
	EXPECT_EQ(request.get_header("accept")->get_value(), "application/json");
	EXPECT_EQ(request.get_header("connection")->get_value(), "close");
	EXPECT_EQ(request.get_content(), std::string{});
}

TEST_F(TestHttpRequestParser,
ParseComplicatedRequest) {
	StringStream stream(
		"POST /endpoint?arg=value%20with%20space&arg2=value2&arg3=%41%20%41 HTTP/1.1\r\n"
		"Accept: application/json\r\n"
		"Connection: close\r\n"
		"Content-Length: 12\r\n"
		"\r\n"
		"Hello World!"
	);

	HttpRequestParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "POST");
	EXPECT_EQ(request.get_resource(), "/endpoint");
	EXPECT_EQ(request.get_arguments().size(), 3u);
	EXPECT_EQ(request.get_argument("arg")->get_value(), "value with space");
	EXPECT_EQ(request.get_argument("arg2")->get_value(), "value2");
	EXPECT_EQ(request.get_argument("arg3")->get_value(), "A A");
	EXPECT_EQ(request.get_headers().size(), 3u);
	EXPECT_EQ(request.get_header("accept")->get_value(), "application/json");
	EXPECT_EQ(request.get_header("connection")->get_value(), "close");
	EXPECT_EQ(request.get_header("content-length")->get_value_as<std::uint64_t>(), 12u);
	EXPECT_EQ(request.get_content(), "Hello World!");
}

TEST_F(TestHttpRequestParser,
ParseByParts) {
	using namespace std::literals;

	StringStream stream(1024);
	HttpRequestParser parser;

	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("GET"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(" /endpoint "sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("HTTP/1.1\r"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\nConnect"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("ion"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(": close"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\r\nContent-"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("Length: 5\r\n\r"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\nHe"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("llo World"sv);
	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto request = result.value();
	EXPECT_EQ(request.get_method(), "GET");
	EXPECT_EQ(request.get_resource(), "/endpoint");
	EXPECT_EQ(request.get_arguments().size(), 0u);
	EXPECT_EQ(request.get_headers().size(), 2u);
	EXPECT_EQ(request.get_header("connection")->get_value(), "close");
	EXPECT_EQ(request.get_header("content-length")->get_value_as<std::uint64_t>(), 5u);
	EXPECT_EQ(request.get_content(), "Hello");
}
