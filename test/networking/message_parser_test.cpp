
#include "gtest/gtest.h"
#include "networking/message_parser.h"
#include <string>

TEST(MessageParserTest, ExtractHttpRequestTest) {
  std::string raw_request = "GET / HTTP/1.1\r\n"
                            "Host: 0.0.0.0:8080\r\n"
                            "User-Agent: python-requests/2.22.0\r\n"
                            "Accept-Encoding: gzip, deflate\r\n"
                            "Accept: */*\r\n"
                            "Connection: keep-alive\r\n\r\n"
                            "Hello";
  MessageParser mp;
  auto http_request = mp.ToHttpRequest(raw_request);

  EXPECT_EQ(HttpMethod::GET, http_request.GetStartLine().method_);

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