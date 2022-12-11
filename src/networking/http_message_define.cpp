#include <algorithm>
#include <networking/http_message_define.h>
#include <string>

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

auto ToString(const HttpVersion &version) -> std::string {
  switch (version) {
    case HttpVersion::HTTP_0_9:
      return "HTTP/0.9";
    case HttpVersion::HTTP_1_0:
      return "HTTP/1.0";
    case HttpVersion::HTTP_1_1:
      return "HTTP/1.1";
    case HttpVersion::HTTP_2_0:
      return "HTTP/2.0";
    default:
      return std::string();
  }
}

auto ToString(const HttpStatusCode &status_code) -> std::string {
  switch (status_code) {
    case HttpStatusCode::CONTINUE:
      return "Continue";
    case HttpStatusCode::OK:
      return "OK";
    case HttpStatusCode::ACCEPTED:
      return "Accepted";
    case HttpStatusCode::MOVE_PERMANENTLY:
      return "Moved Permanently";
    case HttpStatusCode::BAD_REQUEST:
      return "Bad Request";
    case HttpStatusCode::FORBIDDEN:
      return "Forbidden";
    case HttpStatusCode::NOTFOUND:
      return "Not Found";
    case HttpStatusCode::INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    case HttpStatusCode::NOT_IMPLEMENTED:
      return "Not Implemented";
    case HttpStatusCode::BAD_GATEWAY:
      return "Bad Gateway";
    default:
      return std::string();
  }
}