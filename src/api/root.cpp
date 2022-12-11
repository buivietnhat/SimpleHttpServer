#include "api/root.h"

auto Greeting() -> HttpResponse {
  HttpResponseBuilder builder;
  auto response = builder.SetStatusCode(HttpStatusCode::OK)
      .SetHttpVersion(HttpVersion::HTTP_1_1)
      .AddHeaderKeyValue("Content-Type", "text/plain")
      .SetContent("Hello from the other side ~~~")
      .Build();
  return response;
}