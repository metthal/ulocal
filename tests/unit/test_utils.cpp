#include <gtest/gtest.h>

#include <ulocal/utils.hpp>

using namespace ::testing;
using namespace ulocal;

class TestUtils : public ::testing::Test {};

TEST_F(TestUtils,
HexToNibble) {
	EXPECT_EQ(hex_to_nibble('0'), 0);
	EXPECT_EQ(hex_to_nibble('1'), 1);
	EXPECT_EQ(hex_to_nibble('2'), 2);
	EXPECT_EQ(hex_to_nibble('3'), 3);
	EXPECT_EQ(hex_to_nibble('4'), 4);
	EXPECT_EQ(hex_to_nibble('5'), 5);
	EXPECT_EQ(hex_to_nibble('6'), 6);
	EXPECT_EQ(hex_to_nibble('7'), 7);
	EXPECT_EQ(hex_to_nibble('8'), 8);
	EXPECT_EQ(hex_to_nibble('9'), 9);
	EXPECT_EQ(hex_to_nibble('A'), 10);
	EXPECT_EQ(hex_to_nibble('B'), 11);
	EXPECT_EQ(hex_to_nibble('C'), 12);
	EXPECT_EQ(hex_to_nibble('D'), 13);
	EXPECT_EQ(hex_to_nibble('E'), 14);
	EXPECT_EQ(hex_to_nibble('F'), 15);
	EXPECT_EQ(hex_to_nibble('a'), 10);
	EXPECT_EQ(hex_to_nibble('b'), 11);
	EXPECT_EQ(hex_to_nibble('c'), 12);
	EXPECT_EQ(hex_to_nibble('d'), 13);
	EXPECT_EQ(hex_to_nibble('e'), 14);
	EXPECT_EQ(hex_to_nibble('f'), 15);
}

TEST_F(TestUtils,
NibblesToChar) {
	EXPECT_EQ(nibbles_to_char('4', '1'), 'A');
	EXPECT_EQ(nibbles_to_char('6', '1'), 'a');
	EXPECT_EQ(nibbles_to_char('2', '0'), ' ');
	EXPECT_EQ(nibbles_to_char('4', 'F'), 'O');
	EXPECT_EQ(nibbles_to_char('4', 'f'), 'O');
	EXPECT_EQ(nibbles_to_char('0', '0'), '\0');
	EXPECT_EQ(nibbles_to_char('f', 'F'), '\xFF');
}

TEST_F(TestUtils,
UrlDecode) {
	using namespace std::literals;

	EXPECT_EQ(url_decode("abc"sv), "abc");
	EXPECT_EQ(url_decode("ab%20cd"sv), "ab cd");
	EXPECT_EQ(url_decode("ab%20"sv), "ab ");
	EXPECT_EQ(url_decode("%20ab"sv), " ab");
	EXPECT_EQ(url_decode("%20ab%20"sv), " ab ");
	//EXPECT_EQ(url_decoe(), 'a');
	//EXPECT_EQ(url_decoe(), ' ');
	//EXPECT_EQ(url_decoe(), 'O');
	//EXPECT_EQ(url_decoe(), 'O');
	//EXPECT_EQ(url_decoe(), '\0');
	//EXPECT_EQ(url_decoe(), '\xFF');
}
