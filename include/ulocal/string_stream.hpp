#pragma once

#include <cstring>
#include <optional>
#include <string_view>
#include <vector>

namespace ulocal {

class StringStream
{
public:
	StringStream(std::size_t capacity) : _buffer(capacity, 0), _used(0), _read_pos(0) {}
	StringStream(const std::string& str) : _buffer(str.begin(), str.end()), _used(_buffer.size()), _read_pos(0) {}
	StringStream(const StringStream&) = delete;
	StringStream(StringStream&&) noexcept = default;

	StringStream& operator=(const StringStream&) = delete;
	StringStream& operator=(StringStream&&) noexcept = default;

	std::size_t get_capacity() const { return _buffer.size(); }
	std::size_t get_size() const { return _used - _read_pos; }
	std::size_t get_writable_size() const { return get_capacity() - _used; }

	char* get_writable_buffer() { return _buffer.data() + _used; }

	void increase_used(std::size_t count)
	{
		_used = std::min(_used + count, get_capacity());
	}

	std::string_view as_string_view(std::size_t count = 0) const
	{
		return std::string_view{_buffer.data() + _read_pos, count == 0 ? get_size() : std::min(count, get_size())};
	}

	std::optional<char> lookahead_byte() const
	{
		auto sv = as_string_view(1);
		if (sv.empty())
			return std::nullopt;
		return sv[0];
	}

	template <typename T>
	std::size_t lookahead(const T& what) const
	{
		return as_string_view().find(what);
	}

	void skip(std::size_t count)
	{
		count = std::min(count, get_size());
		_read_pos += count;
	}

	std::string_view read(std::size_t count = 0)
	{
		if (count == 0)
			count = get_size();
		else
			count = std::min(count, get_size());
		auto result = std::string_view{_buffer.data() + _read_pos, count};
		_read_pos += count;
		return result;
	}

	std::pair<std::string_view, bool> read_until(char what)
	{
		bool found_delim = true;
		auto pos = lookahead(what);
		if (pos == std::string::npos)
		{
			pos = get_size();
			found_delim = false;
		}
		else if (pos == 0)
			return { std::string_view{}, true };

		return { read(pos), found_delim };
	}

	std::pair<std::string_view, bool> read_until(std::string_view what)
	{
		bool found_delim = true;
		auto pos = lookahead(what);
		if (pos == std::string::npos)
		{
			found_delim = false;

			// Try to find at least the first character and read until then
			// while still reporting that we didn't find delimiter
			pos = lookahead(what[0]);
			if (pos == std::string::npos)
				pos = get_size();
			else // Compare prefix
			{
				auto count = std::min(what.length(), get_size() - pos);
				for (std::size_t i = 1; i < count; ++i)
				{
					// Not a viable prefix so read it whole
					if (what[i] != _buffer[pos + i])
					{
						pos = get_size();
						break;
					}
				}
			}
		}
		else if (pos == 0)
			return { std::string_view{}, true };

		return { read(pos), found_delim };
	}

	template <typename StrT>
	void write_string(const StrT& str)
	{
		std::size_t count = std::min(str.length(), get_writable_size());
		std::memcpy(get_writable_buffer(), str.data(), count);
		increase_used(count);
	}

	void realign()
	{
		if (_read_pos < _used)
			std::memmove(_buffer.data(), _buffer.data() + _read_pos, get_size());
		_used -= _read_pos;
		_read_pos = 0;
	}

private:
	std::vector<char> _buffer;
	std::size_t _used;
	std::size_t _read_pos;
};

} // namespace ulocal
