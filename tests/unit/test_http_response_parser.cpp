#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ulocal/http_response_parser.hpp>

using namespace ::testing;
using namespace ulocal;

class TestHttpResponseParser : public ::testing::Test {};

TEST_F(TestHttpResponseParser,
ParseSimpleResponse) {
	StringStream stream(
		"HTTP/1.1 200 OK\r\n"
		"\r\n"
	);

	HttpResponseParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto response = result.value();
	EXPECT_EQ(response.get_status_code(), 200);
	EXPECT_EQ(response.get_reason(), "OK");
	EXPECT_EQ(response.get_headers().size(), 0u);
	EXPECT_EQ(response.get_content(), std::string{});
}

TEST_F(TestHttpResponseParser,
ParseResponseWithHeaders) {
	StringStream stream(
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\r\n"
		"Server: ulocal\r\n"
		"\r\n"
	);

	HttpResponseParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto response = result.value();
	EXPECT_EQ(response.get_status_code(), 200);
	EXPECT_EQ(response.get_reason(), "OK");
	EXPECT_EQ(response.get_headers().size(), 2u);
	EXPECT_EQ(response.get_content(), std::string{});
}

TEST_F(TestHttpResponseParser,
ParseResponseWithContent) {
	StringStream stream(
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 12\r\n"
		"\r\n"
		"Hello World!"
	);

	HttpResponseParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto response = result.value();
	EXPECT_EQ(response.get_status_code(), 200);
	EXPECT_EQ(response.get_reason(), "OK");
	EXPECT_EQ(response.get_headers().size(), 1u);
	EXPECT_EQ(response.get_header("content-length")->get_value_as<std::uint64_t>(), 12u);
	EXPECT_EQ(response.get_content(), "Hello World!");
}

TEST_F(TestHttpResponseParser,
ParseResponseWithCustomReason) {
	StringStream stream(
		"HTTP/1.1 200 Custom\r\n"
		"\r\n"
	);

	HttpResponseParser parser;

	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto response = result.value();
	EXPECT_EQ(response.get_status_code(), 200);
	EXPECT_EQ(response.get_reason(), "Custom");
	EXPECT_EQ(response.get_headers().size(), 0u);
	EXPECT_EQ(response.get_content(), std::string{});
}

TEST_F(TestHttpResponseParser,
ParseByParts) {
	using namespace std::literals;

	StringStream stream(1024);
	HttpResponseParser parser;

	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("HTTP/1.1"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(" 40"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("0"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(" Bad"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(" Request\r\n"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("Server:"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string(" ulocal\r"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\nContent-"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("Length: 1"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("2\r\n"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\r"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("\n"sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("Hello "sv);
	ASSERT_FALSE(parser.parse(stream));

	stream.write_string("World!!!"sv);
	auto result = parser.parse(stream);
	ASSERT_TRUE(result);

	auto response = result.value();
	EXPECT_EQ(response.get_status_code(), 400);
	EXPECT_EQ(response.get_reason(), "Bad Request");
	EXPECT_EQ(response.get_headers().size(), 2u);
	EXPECT_EQ(response.get_header("server")->get_value(), "ulocal");
	EXPECT_EQ(response.get_header("content-length")->get_value_as<std::uint64_t>(), 12u);
	EXPECT_EQ(response.get_content(), "Hello World!");

}
