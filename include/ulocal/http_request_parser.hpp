#pragma once

#include <string>

#include <ulocal/http_header_table.hpp>
#include <ulocal/http_request.hpp>
#include <ulocal/string_stream.hpp>
#include <ulocal/url_args.hpp>

namespace ulocal {

namespace detail {

enum class RequestState
{
	Start,
	StatusLineMethod,
	StatusLineResource,
	StatusLineHttpVersion,
	HeaderName,
	HeaderValue,
	Content
};

} // namespace detail

class HttpRequestParser
{
public:
	HttpRequestParser() : _state(detail::RequestState::Start) {}
	HttpRequestParser(const HttpRequestParser&) = delete;
	HttpRequestParser(HttpRequestParser&&) noexcept = default;

	HttpRequestParser& operator=(const HttpRequestParser&) = delete;
	HttpRequestParser& operator=(HttpRequestParser&&) noexcept = default;

	std::optional<HttpRequest> parse(StringStream& stream)
	{
		bool continue_parsing = true;
		while (continue_parsing)
		{
			switch (_state)
			{
				case detail::RequestState::Start:
				{
					_method.clear();
					_resource.clear();
					_http_version.clear();
					_header_name.clear();
					_header_value.clear();
					_headers.clear();
					_content.clear();
					_content_length = 0;
					_state = detail::RequestState::StatusLineMethod;
					break;
				}
				case detail::RequestState::StatusLineMethod:
				{
					auto [str, found_space] = stream.read_until(' ');
					_method += str;
					if (found_space)
					{
						_state = detail::RequestState::StatusLineResource;
						stream.skip(1);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::RequestState::StatusLineResource:
				{
					auto [str, found_space] = stream.read_until(' ');
					_resource += str;
					if (found_space)
					{
						_state = detail::RequestState::StatusLineHttpVersion;
						stream.skip(1);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::RequestState::StatusLineHttpVersion:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_http_version += str;
					if (found_newline)
					{
						_state = detail::RequestState::HeaderName;
						stream.skip(2);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::RequestState::HeaderName:
				{
					if (stream.as_string_view(2) == "\r\n")
					{
						stream.skip(2);
						_state = detail::RequestState::Content;

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
							_state = detail::RequestState::HeaderValue;
							stream.skip(1);
						}
						else
							continue_parsing = false;
					}
					break;
				}
				case detail::RequestState::HeaderValue:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_header_value += str;
					if (found_newline)
					{
						_state = detail::RequestState::HeaderName;
						stream.skip(2);
						_headers.add_header(std::move(_header_name), lstrip(_header_value));
						_header_name.clear();
						_header_value.clear();
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::RequestState::Content:
				{
					auto str = stream.read(_content_length - _content.length());
					_content += str;
					if (_content.length() == _content_length)
					{
						_state = detail::RequestState::Start;
						return HttpRequest{
							std::move(_method),
							std::move(_resource),
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
	detail::RequestState _state;
	std::string _method, _resource, _http_version, _header_name, _header_value, _content;
	HttpHeaderTable _headers;
	std::uint64_t _content_length;
};

} // namespace ulocal
