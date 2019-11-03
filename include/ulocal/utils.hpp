#pragma once

#include <algorithm>

namespace ulocal {

inline char hex_to_nibble(char c)
{
	auto lc = std::tolower(c);
	if ('0' <= lc && lc <= '9')
		return (c - '0') & 0x0F;
	else if ('a' <= lc && lc <= 'f')
		return (c - 'a' + 10) & 0x0F;
	else
		throw std::runtime_error("Invalid nibble value");
}

inline char nibbles_to_char(char high, char low)
{
	return (hex_to_nibble(high) << 4) | hex_to_nibble(low);
}

template <typename StrT>
std::string url_decode(const StrT& str)
{
	std::string result;
	result.reserve(str.length());

	auto pos = str.find('%');
	decltype(pos) old_pos = 0;
	while (pos != std::string::npos)
	{
		if (pos > old_pos)
			result.append(str, old_pos, pos - old_pos);

		if (pos + 2 >= str.length())
		{
			break;
		}
		else
		{
			result += nibbles_to_char(str[pos + 1], str[pos + 2]);
			old_pos = std::min(str.length(), pos + 3);
			pos = str.find('%', old_pos);
		}
	}

	if (old_pos != str.length())
		result.append(str, old_pos);

	return result;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename StrT1, typename StrT2>
bool icase_compare(const StrT1& str1, const StrT2& str2)
{
	return std::equal(
		std::begin(str1), std::end(str1),
		std::begin(str2), std::end(str2),
		[](auto c1, auto c2) { return std::tolower(c1) == std::tolower(c2); }
	);
}

template <typename StrT>
std::string lstrip(const StrT& str)
{
	auto pos = str.find_first_not_of(" \r\n\t\v");
	if (pos == std::string::npos)
		return str;
	return std::string{str.data() + pos, str.length() - pos};
}

struct CaseInsensitiveHash
{
	std::size_t operator()(const std::string& str) const
	{
		std::size_t seed = 0;
		for (auto c : str)
			hash_combine(seed, std::tolower(c));
		return seed;
	}
};

struct CaseInsensitiveCompare
{
	bool operator()(const std::string& str1, const std::string& str2) const
	{
		return icase_compare(str1, str2);
	}
};

} // namespace ulocal
