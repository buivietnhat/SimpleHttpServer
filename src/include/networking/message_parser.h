
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

class AbstractHttpMessage {
 public:
  AbstractHttpMessage();
  virtual ~AbstractHttpMessage() = default;

  auto GetStartLine() const -> const StartLine &;
  void SetStartLine(const StartLine &start_line);
  auto GetHeader() const -> const Header &;
  void SetHeader(const Header &header);
  auto GetBody() const -> const Body &;
  void SetBody(const Body &body);
  virtual auto ToString() -> std::string = 0;

 protected:
  StartLine start_line_;
  Header header_;
  Body body_;
};

class HttpRequest : public AbstractHttpMessage {
 public:
  HttpRequest(const std::string &raw);
  auto ToString() -> std::string override;

 private:
  auto ExtractStartLine(const std::string &line) -> StartLine;
  auto ExtractHeader(const std::string &header) -> Header;
  auto ExtractBody(const std::string &body) -> Body;

  std::string raw_;
};

class HttpResponse : public AbstractHttpMessage {
 public:
  auto GetStatusCode() const -> HttpStatusCode;
  void SetStatusCode(HttpStatusCode status_code);
  auto ToString() -> std::string override;

 private:
  HttpStatusCode status_code_;
};

#endif//SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_MESSAGE_PARSER_H_
