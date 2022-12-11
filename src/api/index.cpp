
#include "api/index.h"

auto RenderIndexHtmL() -> HttpResponse {
  HttpResponseBuilder builder;

  std::string html_content = "<!DOCTYPE html>\n"
                             "<html>\n"
                             "<body>\n"
                             "\n"
                             "<h1>My Heading</h1>\n"
                             "<p>My paragraph.</p>\n"
                             "\n"
                             "</body>\n"
                             "</html>";

  auto response = builder.SetStatusCode(HttpStatusCode::OK)
                      .SetHttpVersion(HttpVersion::HTTP_1_1)
                      .AddHeaderKeyValue("Content-Type", "text/html")
                      .SetContent(html_content)
                      .Build();
  return response;
}