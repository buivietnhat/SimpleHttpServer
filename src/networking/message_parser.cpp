
#include "include/networking/message_parser.h"
#include <sstream>

HttpMessageAbstract::HttpMessageAbstract() {
}

const StartLine &HttpMessageAbstract::GetStartLine() const {
  return start_line_;
}

void HttpMessageAbstract::SetStartLine(const StartLine &start_line) {
  start_line_ = start_line;
}

const Header &HttpMessageAbstract::GetHeader() const {
  return header_;
}

void HttpMessageAbstract::SetHeader(const Header &header) {
  header_ = header;
}

const Body &HttpMessageAbstract::GetBody() const {
  return body_;
}

void HttpMessageAbstract::SetBody(const Body &body) {
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
    throw std::invalid_argument("start line not found");
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

std::string HttpRequest::ToString() {
  return raw_;
}

auto HttpRequest::ExtractStartLine(const std::string &line) -> StartLine {
  std::istringstream iss(line);
  std::string method, request_target, version;
  iss >> method >> request_target >> version;
  if (!iss.good() && !iss.eof()) {
    throw std::invalid_argument("invalid start line format");
  }

  StartLine sl;
  sl.method_ = ToMethod(method);
  sl.request_target_ = request_target;
  sl.version_ = ToVersion(version);
  if (sl.version_ != HttpVersion::HTTP_1_1) {
    throw std::logic_error("HTTP version not supported");
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

auto MessageParser::ToHttpRequest(const std::string &raw) -> HttpRequest {
  auto http_request = HttpRequest(raw);
  return http_request;
}
