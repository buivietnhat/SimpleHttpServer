
#ifndef SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_MESSAGE_PARSER_H_
#define SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_MESSAGE_PARSER_H_

#include "networking/http_message_define.h"
#include <algorithm>
#include <string>
#include <unordered_map>

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages

struct StartLine {
  HttpMethod method_;
  std::string request_target_;
  HttpVersion version_;
};

struct Header {
  std::unordered_map<std::string, std::string> header_;
  void SetContentLength(int length) {
    header_["Content-Length"] = std::to_string(length);
  }
};

// for now only support single-resource body
struct Body {
  std::string content_;
};

class HttpMessageAbstract {
 public:
  HttpMessageAbstract();
  virtual ~HttpMessageAbstract() = default;

  const StartLine &GetStartLine() const;
  void SetStartLine(const StartLine &start_line);
  const Header &GetHeader() const;
  void SetHeader(const Header &header);
  const Body &GetBody() const;
  void SetBody(const Body &body);

  virtual auto ToString() -> std::string = 0;

 protected:
  StartLine start_line_;
  Header header_;
  Body body_;
};

class HttpRequest : public HttpMessageAbstract {
 public:
  HttpRequest(const std::string &raw);
  auto ToString() -> std::string override;

 private:
  auto ExtractStartLine(const std::string &line) -> StartLine;
  auto ExtractHeader(const std::string &header) -> Header;
  auto ExtractBody(const std::string &body) -> Body;

  const std::string raw_;
};

class HttpResponse : public HttpMessageAbstract {
};

class MessageParser {
 public:
  MessageParser() = default;
  auto ToHttpRequest(const std::string &raw) -> HttpRequest;
  auto ToHttpResponse(const std::string &raw) -> HttpResponse;
};

inline auto ToMethod(const std::string &method) -> HttpMethod {
  // first to convert to upper case
  std::string upper_method = method;
  std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(), ::toupper);

  if (upper_method == "GET") {
    return HttpMethod::GET;
  } else if (upper_method == "HEAD") {
    return HttpMethod::HEAD;
  } else if (upper_method == "POST") {
    return HttpMethod::POST;
  } else if (upper_method == "PUT") {
    return HttpMethod::PUT;
  } else if (upper_method == "DELETE") {
    return HttpMethod::DELETE;
  } else if (upper_method == "CONNECT") {
    return HttpMethod::CONNECT;
  } else if (upper_method == "OPTIONS") {
    return HttpMethod::OPTIONS;
  } else if (upper_method == "TRACE") {
    return HttpMethod::TRACE;
  } else if (upper_method == "PATCH") {
    return HttpMethod::PATCH;
  } else {
    throw std::invalid_argument("not supported HTTP method");
  }
}

inline auto ToVersion(const std::string &version) -> HttpVersion {
  // first to convert to upper case
  std::string upper_version = version;
  std::transform(upper_version.begin(), upper_version.end(), upper_version.begin(), ::toupper);

  if (upper_version == "HTTP/0.9") {
    return HttpVersion::HTTP_0_9;
  } else if (upper_version == "HTTP/1.0") {
    return HttpVersion::HTTP_1_0;
  } else if (upper_version == "HTTP/1.1") {
    return HttpVersion::HTTP_1_1;
  } else if (upper_version == "HTTP/2" || upper_version == "HTTP/2.0") {
    return HttpVersion::HTTP_2_0;
  } else {
    throw std::invalid_argument("not supported HTTP version");
  }
}

#endif//SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_MESSAGE_PARSER_H_
