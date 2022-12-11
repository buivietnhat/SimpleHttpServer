
#include "networking/http_server.h"
#include "networking/http_message_define.h"
#include "gtest/gtest.h"
#include <string>

TEST(MessageParserTest, ExtractHttpRequestTest) {
  std::string raw_request = "GET / HTTP/1.1\r\n"
                            "Host: 0.0.0.0:8080\r\n"
                            "User-Agent: python-requests/2.22.0\r\n"
                            "Accept-Encoding: gzip, deflate\r\n"
                            "Accept: */*\r\n"
                            "Connection: keep-alive\r\n\r\n"
                            "Hello";
  HttpServer::MessageParser mp;
  auto http_request = mp.ToHttpRequest(raw_request);

  EXPECT_EQ(HttpMethod::GET, http_request.GetStartLine().method_);
  EXPECT_EQ("/", http_request.GetStartLine().request_target_);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http_request.GetStartLine().version_);

  auto header = http_request.GetHeader();
  EXPECT_TRUE(header.header_.count("Host") == 1);
  EXPECT_EQ(" 0.0.0.0:8080", header.header_["Host"]);

  EXPECT_TRUE(header.header_.count("User-Agent") == 1);
  EXPECT_EQ(" python-requests/2.22.0", header.header_["User-Agent"]);

  EXPECT_TRUE(header.header_.count("Accept-Encoding") == 1);
  EXPECT_EQ(" gzip, deflate", header.header_["Accept-Encoding"]);

  EXPECT_TRUE(header.header_.count("Connection") == 1);
  EXPECT_EQ(" keep-alive", header.header_["Connection"]);

  auto body = http_request.GetBody();
  EXPECT_EQ("Hello", body.content_);
}

TEST(MessageParserTest, HttpResponseBuilderTest) {
  HttpResponseBuilder builder;
  auto http_respone = builder.SetStatusCode(HttpStatusCode::OK)
                          .SetHttpVersion(HttpVersion::HTTP_1_1)
                          .AddHeaderKeyValue("Content-Type", "text/plain")
                          .SetContent("Hello")
                          .Build();
  EXPECT_EQ(HttpStatusCode::OK, http_respone.GetStatusCode());
  EXPECT_EQ(HttpVersion::HTTP_1_1, http_respone.GetStartLine().version_);

  auto header = http_respone.GetHeader();
  EXPECT_TRUE(header.header_.count("Content-Type") == 1);
  EXPECT_EQ("text/plain", header.header_["Content-Type"]);

  auto body = http_respone.GetBody();
  EXPECT_EQ("Hello", body.content_);
}

TEST(MessageParserTest, HttpResponseToString) {
  HttpResponseBuilder builder;
  auto http_respone = builder.SetStatusCode(HttpStatusCode::OK)
                          .SetHttpVersion(HttpVersion::HTTP_1_1)
                          .AddHeaderKeyValue("Content-Type", "text/plain")
                          .SetContent("Hello")
                          .Build();

  std::string expected_string_with_content = "HTTP/1.1 200 OK\r\n"
                                             "Content-Length: 5\r\n"
                                             "Content-Type: text/plain\r\n\r\n"
                                             "Hello";
  std::string expected_string_no_content = "HTTP/1.1 200 OK\r\n"
                                             "Content-Length: 5\r\n"
                                             "Content-Type: text/plain\r\n\r\n";

  auto string_response_with_content = http_respone.ToString(true);
  auto string_response_no_content = http_respone.ToString(false);

  EXPECT_EQ(expected_string_with_content, string_response_with_content);
  EXPECT_EQ(expected_string_no_content, string_response_no_content);
}