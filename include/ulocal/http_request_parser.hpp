#pragma once

#include <string>

#include <ulocal/http_header_table.hpp>
#include <ulocal/http_request.hpp>
#include <ulocal/string_stream.hpp>
#include <ulocal/url_args.hpp>

namespace ulocal {

namespace detail {

enum class State
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
	HttpRequestParser() : _state(detail::State::Start) {}
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
				case detail::State::Start:
				{
					_method.clear();
					_resource.clear();
					_args.clear();
					_http_version.clear();
					_header_name.clear();
					_header_value.clear();
					_headers.clear();
					_content.clear();
					_content_length = 0;
					_state = detail::State::StatusLineMethod;
					break;
				}
				case detail::State::StatusLineMethod:
				{
					auto [str, found_space] = stream.read_until(' ');
					_method += str;
					if (found_space)
					{
						_state = detail::State::StatusLineResource;
						stream.skip(1);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::State::StatusLineResource:
				{
					auto [str, found_space] = stream.read_until(' ');
					_resource += str;
					if (found_space)
					{
						_state = detail::State::StatusLineHttpVersion;
						stream.skip(1);

						if (auto url_params_start = _resource.find('?'); url_params_start != std::string::npos)
						{
							auto url_args = std::string_view{_resource.data() + url_params_start + 1, _resource.length() - url_params_start - 1};
							auto pos  = url_args.find('&');
							decltype(pos) old_pos = 0;
							while (pos != std::string::npos)
							{
								auto arg = std::string_view{url_args.data() + old_pos, pos - old_pos};
								auto [key, value] = parse_url_arg(arg);
								_args.add_arg(std::move(key), std::move(value));
								old_pos = pos + 1;
								pos = url_args.find('&', old_pos);
							}

							auto last_arg = std::string_view{url_args.data() + old_pos, url_args.length() - old_pos};
							auto [key, value] = parse_url_arg(last_arg);
							_args.add_arg(std::move(key), std::move(value));

							_resource.erase(url_params_start);
						}
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::State::StatusLineHttpVersion:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_http_version += str;
					if (found_newline)
					{
						_state = detail::State::HeaderName;
						stream.skip(2);
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::State::HeaderName:
				{
					if (stream.as_string_view(2) == "\r\n")
					{
						stream.skip(2);
						_state = detail::State::Content;

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
							_state = detail::State::HeaderValue;
							stream.skip(1);
						}
						else
							continue_parsing = false;
					}
					break;
				}
				case detail::State::HeaderValue:
				{
					auto [str, found_newline] = stream.read_until("\r\n");
					_header_value += str;
					if (found_newline)
					{
						_state = detail::State::HeaderName;
						stream.skip(2);
						_headers.add_header(std::move(_header_name), lstrip(_header_value));
						_header_name.clear();
						_header_value.clear();
					}
					else
						continue_parsing = false;
					break;
				}
				case detail::State::Content:
				{
					auto str = stream.read(_content_length - _content.length());
					_content += str;
					if (_content.length() == _content_length)
					{
						_state = detail::State::Start;
						return HttpRequest{
							std::move(_method),
							std::move(_resource),
							std::move(_args),
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
	std::pair<std::string, std::string> parse_url_arg(const std::string_view& arg)
	{
		auto value_pos = arg.find('=');
		if (value_pos == std::string::npos)
			return { std::string{arg}, std::string{} };

		return {
			url_decode(std::string_view{arg.data(), value_pos}),
			url_decode(std::string_view{arg.data() + value_pos + 1, arg.length() - value_pos - 1})
		};
	}

	detail::State _state;
	std::string _method, _resource, _http_version, _header_name, _header_value, _content;
	HttpHeaderTable _headers;
	UrlArgs _args;
	std::uint64_t _content_length;
};

} // namespace ulocal
