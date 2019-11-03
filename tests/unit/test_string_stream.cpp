#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ulocal/string_stream.hpp>

using namespace ::testing;
using namespace ulocal;

class TestStringStream : public ::testing::Test {};

TEST_F(TestStringStream,
InitEmpty) {
	StringStream stream(10);

	EXPECT_EQ(stream.get_capacity(), 10u);
	EXPECT_EQ(stream.get_size(), 0u);
	EXPECT_EQ(stream.get_writable_size(), 10u);
}

TEST_F(TestStringStream,
InitFromString) {
	StringStream stream("abc");

	EXPECT_EQ(stream.get_capacity(), 3u);
	EXPECT_EQ(stream.get_size(), 3u);
	EXPECT_EQ(stream.get_writable_size(), 0u);
}

TEST_F(TestStringStream,
WritableBuffer) {
	StringStream stream(10);

	std::memcpy(stream.get_writable_buffer(), "Hello World!", stream.get_writable_size());
	stream.increase_used(std::min(std::strlen("Hello World!"), stream.get_writable_size()));

	EXPECT_EQ(stream.get_capacity(), 10u);
	EXPECT_EQ(stream.get_size(), 10u);
	EXPECT_EQ(stream.get_writable_size(), 0u);
	EXPECT_EQ(stream.as_string_view(), "Hello Worl");
}

TEST_F(TestStringStream,
Read) {
	StringStream stream("abcdef");

	EXPECT_EQ(stream.read(4), "abcd");
	EXPECT_EQ(stream.read(4), "ef");
	EXPECT_EQ(stream.read(4), std::string_view{});
}

TEST_F(TestStringStream,
ReadUntilChar) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until('d'), Pair(Eq("abc"), Eq(true)));
	EXPECT_THAT(stream.read_until('x'), Pair(Eq("def"), Eq(false)));
	EXPECT_THAT(stream.read_until('f'), Pair(Eq(std::string_view{}), Eq(false)));
}

TEST_F(TestStringStream,
ReadUntilStringFound) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until("ef"), Pair(Eq("abcd"), Eq(true)));
}

TEST_F(TestStringStream,
ReadUntilStringNotFound) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until("xy"), Pair(Eq("abcdef"), Eq(false)));
}

TEST_F(TestStringStream,
ReadUntilStringPartialMatch) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until("efg"), Pair(Eq("abcd"), Eq(false)));
}

TEST_F(TestStringStream,
ReadUntilStringPartialNonMatch) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until("exy"), Pair(Eq("abcdef"), Eq(false)));
}

TEST_F(TestStringStream,
ReadUntilAtTheCharacter) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until('a'), Pair(Eq(std::string_view{}), Eq(true)));
}

TEST_F(TestStringStream,
ReadUntilAtTheString) {
	StringStream stream("abcdef");

	EXPECT_THAT(stream.read_until("ab"), Pair(Eq(std::string_view{}), Eq(true)));
}

TEST_F(TestStringStream,
ReadUntilAtTheCharacterInTheMiddle) {
	StringStream stream("abcdef");

	EXPECT_EQ(stream.read(2), "ab");
	EXPECT_THAT(stream.read_until('c'), Pair(Eq(std::string_view{}), Eq(true)));
}

TEST_F(TestStringStream,
ReadUntilAtTheStringInTheMiddle) {
	StringStream stream("abcdef");

	EXPECT_EQ(stream.read(2), "ab");
	EXPECT_THAT(stream.read_until("cd"), Pair(Eq(std::string_view{}), Eq(true)));
}

TEST_F(TestStringStream,
Realign) {
	StringStream stream("abcdef");

	stream.read(4);
	stream.realign();

	EXPECT_EQ(stream.get_size(), 2u);
	EXPECT_EQ(stream.get_writable_size(), 4u);
	EXPECT_EQ(stream.read(4), "ef");
}

TEST_F(TestStringStream,
WriteString) {
	using namespace std::literals;

	StringStream stream(8);

	stream.write_string("abc"sv);
	EXPECT_EQ(stream.read(4), "abc");

	stream.write_string("def"sv);
	EXPECT_EQ(stream.read(4), "def");

	stream.realign();
	stream.write_string("xyz"sv);
	EXPECT_EQ(stream.read(4), "xyz");
}
