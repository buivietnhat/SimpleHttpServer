
#ifndef SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_HTTP_MESSAGE_DEFINE_H_
#define SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_HTTP_MESSAGE_DEFINE_H_

#include <string>
#include <algorithm>
#include <string>
#include <unordered_map>

enum class HttpMethod {
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH
};

enum class HttpVersion {
  HTTP_0_9,
  HTTP_1_0,
  HTTP_1_1,
  HTTP_2_0
};

enum class HttpStatusCode {
  CONTINUE = 100,
  SWITCHING_PROTOCOLS = 101,
  EARLY_HINTS = 103,
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  FORBIDDEN = 403,
  NOTFOUND = 404,
  MULTILE_CHOISES = 300,
  MOVE_PERMANENTLY = 301,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
  HTTP_VERSION_NOT_SUPPORTED = 505
};

auto ToMethod(const std::string &method) -> HttpMethod;
auto ToVersion(const std::string &version) -> HttpVersion;
auto ToString(const HttpVersion &version) -> std::string;
auto ToString(const HttpStatusCode &status_code) -> std::string;

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
  virtual auto ToString(bool content_included) const -> std::string = 0;

 protected:
  StartLine start_line_;
  Header header_;
  Body body_;
};

class HttpRequest : public AbstractHttpMessage {
 public:
  HttpRequest(const std::string &raw);
  auto ToString(bool content_included) const -> std::string override;

 private:
  auto ExtractStartLine(const std::string &line) -> StartLine;
  auto ExtractHeader(const std::string &header) -> Header;
  auto ExtractBody(const std::string &body) -> Body;

  std::string raw_;
};

class HttpResponse : public AbstractHttpMessage {
  friend class HttpResponseBuilder;

 public:
  auto GetStatusCode() const -> HttpStatusCode;
  void SetStatusCode(HttpStatusCode status_code);
  auto ToString(bool content_included) const -> std::string override;

 private:
  HttpStatusCode status_code_;
};

class HttpResponseBuilder {
 public:
  auto SetStatusCode(HttpStatusCode code) -> HttpResponseBuilder &;
  auto SetHttpVersion(HttpVersion version) -> HttpResponseBuilder &;
  auto AddHeaderKeyValue(const std::string &key, const std::string &value) -> HttpResponseBuilder &;
  auto SetContent(const std::string &content) -> HttpResponseBuilder &;
  auto Build() -> HttpResponse;

 private:
  HttpResponse root_;
};

#endif//SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_HTTP_MESSAGE_DEFINE_H_
