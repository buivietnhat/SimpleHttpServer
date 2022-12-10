
#include "networking/message_parser.h"
#include "networking/http_server.h"
#include <sstream>

auto ToMethod(const std::string &method) -> HttpMethod;
auto ToVersion(const std::string &version) -> HttpVersion;

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

auto HttpRequest::ToString() -> std::string {
  return raw_;
}

auto HttpServer::MessageParser::ToHttpRequest(const std::string &raw) -> HttpRequest {
  auto http_request = HttpRequest(raw);
  return http_request;
}

auto HttpServer::MessageParser::ToPeerState(const HttpResponse &response) -> HttpServer::PeerState * {
  return new HttpServer::PeerState();
}

auto ToMethod(const std::string &method) -> HttpMethod {
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

auto ToVersion(const std::string &version) -> HttpVersion {
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

auto HttpResponse::ToString() -> std::string {
  return std::string();
}

auto HttpResponse::GetStatusCode() const -> HttpStatusCode {
  return status_code_;
}

void HttpResponse::SetStatusCode(HttpStatusCode status_code) {
  status_code_ = status_code;
}
