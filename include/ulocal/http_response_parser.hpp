#pragma once

#include <string>

#include <ulocal/http_response.hpp>
#include <ulocal/string_stream.hpp>

namespace ulocal {

namespace detail {

enum class ResponseState
{
	Start,
	StatusLineHttpVersion,
	StatusLineCode,
	StatusLineReason,
	HeaderName,
	HeaderValue,
	Content
};

} // namespace detail

class HttpResponseParser
{
public:
	HttpResponseParser() : _state(detail::ResponseState::Start) {}
	HttpResponseParser(const HttpResponseParser&) = delete;
	HttpResponseParser(HttpResponseParser&&) noexcept = default;

	HttpResponseParser& operator=(const HttpResponseParser&) = delete;
	HttpResponseParser& operator=(HttpResponseParser&&) noexcept = default;

	std::optional<HttpResponse> parse(StringStream& stream)
	{
		bool continue_parsing = true;
		while (continue_parsing)
		{
			switch (_state)
			{
				case detail::ResponseState::Start:
					_http_version.clear();
					_status_code.clear();
					_reason.clear();
					_header_name.clear();
					_header_value.clear();
					_content.clear();
					_headers.clear();
					_content_length = 0;
					_state = detail::ResponseState::StatusLineHttpVersion;
					break;
				case detail::ResponseState::StatusLineHttpVersion:
				{
					auto [str, found_space] = stream.read_until(' ');
					_http_version += str;
					if (found_space)
					{
						_state = detail::ResponseState::StatusLineCode;
						stream.skip(1);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::ResponseState::StatusLineCode:
				{
					auto [str, found_space] = stream.read_until(' ');
					_status_code += str;
					if (found_space)
					{
						_state = detail::ResponseState::StatusLineReason;
						stream.skip(1);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::ResponseState::StatusLineReason:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_reason += str;
					if (found_newline)
					{
						_state = detail::ResponseState::HeaderName;
						stream.skip(2);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::ResponseState::HeaderName:
				{
					if (stream.as_string_view(2) == "\r\n")
					{
						stream.skip(2);
						_state = detail::ResponseState::Content;

						auto content_length_header = _headers.get_header("content-length");
						if (content_length_header)
							_content_length = content_length_header->get_value_as<std::uint64_t>();
					}
					else if (stream.as_string_view(1) == "\r")
					{
						continue_parsing = false;
					}
					else
					{
						auto [str, found_colon] = stream.read_until(':');
						_header_name += str;
						if (found_colon)
						{
							_state = detail::ResponseState::HeaderValue;
							stream.skip(1);
						}
						else
							continue_parsing = false;
					}
					break;
				}
				case detail::ResponseState::HeaderValue:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_header_value += str;
					if (found_newline)
					{
						_state = detail::ResponseState::HeaderName;
						stream.skip(2);
						_headers.add_header(std::move(_header_name), lstrip(_header_value));
						_header_name.clear();
						_header_value.clear();
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::ResponseState::Content:
				{
					auto str = stream.read(_content_length - _content.length());
					_content += str;
					if (_content.length() == _content_length)
					{
						_state = detail::ResponseState::Start;
						return HttpResponse{
							std::stoi(_status_code),
							std::move(_reason),
							std::move(_headers),
							std::move(_content)
						};
					}
					else
						continue_parsing = false;
					break;
				}
			}
		}

		stream.realign();
		return std::nullopt;
	}

private:
	detail::ResponseState _state;
	std::string _http_version, _status_code, _reason, _header_name, _header_value, _content;
	HttpHeaderTable _headers;
	std::uint64_t _content_length;
};

} // namespace ulocal
