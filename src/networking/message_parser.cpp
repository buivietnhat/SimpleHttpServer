
#include "networking/http_message_define.h"
#include "networking/http_server.h"
#include <sstream>
#include <cstring>
#include <iostream>

AbstractHttpMessage::AbstractHttpMessage() {
}

auto AbstractHttpMessage::GetStartLine() const -> const StartLine & {
  return start_line_;
}

void AbstractHttpMessage::SetStartLine(const StartLine &start_line) {
  start_line_ = start_line;
}

auto AbstractHttpMessage::GetHeader() const -> const Header & {
  return header_;
}

void AbstractHttpMessage::SetHeader(const Header &header) {
  header_ = header;
}

auto AbstractHttpMessage::GetBody() const -> const Body & {
  return body_;
}

void AbstractHttpMessage::SetBody(const Body &body) {
  body_ = body;
}

//GET / HTTP/1.1
//Host: 0.0.0.0:8080
//User-Agent: python-requests/2.22.0
//Accept-Encoding: gzip, deflate
//Accept: */*
//Connection: keep-alive

HttpRequest::HttpRequest(const std::string &raw) : raw_(raw) {
  size_t left_pos = 0, right_pos = 0;

  // extract the start line
  right_pos = raw_.find("\r\n", left_pos);
  if (right_pos == std::string::npos) {
    throw std::logic_error("start line not found");
  }
  auto start_line = raw.substr(left_pos, right_pos);
  auto sl = ExtractStartLine(start_line);
  SetStartLine(sl);

  // extract header:
  // https://stackoverflow.com/questions/4551898/separating-http-response-body-from-header-in-c
  left_pos = right_pos + 2;
  right_pos = raw.find("\r\n\r\n", left_pos);
  if (right_pos != std::string::npos) {
    auto header = raw.substr(left_pos, right_pos - left_pos);
    auto h = ExtractHeader(header);
    SetHeader(h);

    // extract body
    left_pos = right_pos + 4;
    right_pos = raw_.length();
    if (left_pos < right_pos) {
      auto body = raw.substr(left_pos, right_pos - left_pos);
      auto b = ExtractBody(body);
      SetBody(b);

      // update the content-length header
      header_.SetContentLength(body_.content_.size());
    }
  }
}

auto HttpRequest::ExtractStartLine(const std::string &line) -> StartLine {
  std::istringstream iss(line);
  std::string method, request_target, version;
  iss >> method >> request_target >> version;
  if (!iss.good() && !iss.eof()) {
    throw std::logic_error("invalid start line format");
  }

  StartLine sl;
  sl.method_ = ToMethod(method);
  sl.request_target_ = request_target;
  sl.version_ = ToVersion(version);
  if (sl.version_ != HttpVersion::HTTP_1_1) {
    throw std::invalid_argument("HTTP version not supported");
  }

  return sl;
}

auto HttpRequest::ExtractHeader(const std::string &header) -> Header {
  std::istringstream iss(header);
  std::string line;
  std::unordered_map<std::string, std::string> header_key_value;
  while (std::getline(iss, line)) {
    auto pos = line.find(":");
    auto key = line.substr(0, pos);
    // header ends with \r\n
    auto pos1 = line.find("\r");
    auto value = line.substr(pos + 1, pos1 - pos - 1);
    header_key_value[key] = value;
  }
  Header h;
  h.header_ = std::move(header_key_value);
  return h;
}

auto HttpRequest::ExtractBody(const std::string &body) -> Body {
  Body b;
  b.content_ = body;
  return b;
}

auto HttpRequest::ToString(bool content_included) const -> std::string {
  if (content_included) {
    return raw_;
  }
  throw std::logic_error("function not yet implemented");
}

auto HttpServer::MessageParser::ToHttpRequest(const std::string &raw) -> HttpRequest {
  auto http_request = HttpRequest(raw);
  return http_request;
}

auto HttpServer::MessageParser::ToPeerState(int fd, const HttpResponse &response,
                                            bool content_included) -> HttpServer::PeerState * {
  auto peer_state = new PeerState();
  peer_state->fd = fd;
  auto response_string = response.ToString(content_included);
//  std::string response_string = "HTTP/1.1 200 OK\r\n"
//                                "Content-Length: 13\r\n"
//                                "Content-Type: text/plain\r\n\r\n"
//                                "Hello, world\n";
  memcpy(peer_state->buffer, response_string.c_str(), BUFFER_SIZE);
  peer_state->length = response_string.size();
  return peer_state;
}

auto HttpResponse::ToString(bool content_included) const -> std::string {
  // Start Line
  std::ostringstream oss;

  oss << ::ToString(start_line_.version_) << " ";
  oss << static_cast<int>(status_code_) << " ";
  oss << ::ToString(status_code_) << "\r\n";

  // Header
  for (const auto &[key, value] : header_.header_) {
    oss << key << ": " << value << "\r\n";
  }
  oss << "\r\n";

  // Body
  if (content_included) {
    oss << body_.content_;
  }

  return oss.str();
}

auto HttpResponse::GetStatusCode() const -> HttpStatusCode {
  return status_code_;
}

void HttpResponse::SetStatusCode(HttpStatusCode status_code) {
  status_code_ = status_code;
}

auto HttpResponseBuilder::SetStatusCode(HttpStatusCode code) -> HttpResponseBuilder & {
  root.SetStatusCode(code);
  return *this;
}

auto HttpResponseBuilder::SetHttpVersion(HttpVersion version) -> HttpResponseBuilder & {
  root.start_line_.version_ = version;
  return *this;
}

auto HttpResponseBuilder::AddHeaderKeyValue(const std::string &key, const std::string &value) -> HttpResponseBuilder & {
  root.header_.header_[key] = value;
  return *this;
}

auto HttpResponseBuilder::SetContent(const std::string &content) -> HttpResponseBuilder & {
  root.body_.content_ = content;
  root.header_.SetContentLength(content.size());
  return *this;
}

auto HttpResponseBuilder::Build() -> HttpResponse {
  return std::move(root);
}
