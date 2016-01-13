#include <unistd.h>

#include <iostream>

#include "http/http_callbacks.h"
#include "http/http_client.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include "base/end_point.h"
#include "base/ip_address.h"

int main(int argc, const char **argv) {
  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
    return 1;
  }

  using HttpConnectionPtr = std::shared_ptr<cnetpp::http::HttpConnection>;

  cnetpp::http::HttpClientOptions http_options;
  http_options.set_send_buffer_size(1024 * 1024);
  http_options.set_receive_buffer_size(1024 * 1024);

  auto send_func = [] (HttpConnectionPtr c) -> bool {
    std::shared_ptr<cnetpp::http::HttpRequest> http_request(
        new cnetpp::http::HttpRequest);
    http_request->set_method(cnetpp::http::HttpRequest::MethodType::kGet);
    http_request->set_uri("/");
    // http_request->SetHttpHeader("Content-Length", "0");
    if (!c->remote_hostname().empty()) {
      http_request->SetHttpHeader("Host", c->remote_hostname());
    } else {
      auto tc = c->tcp_connection();
      http_request->SetHttpHeader(
          "Host",
          tc->remote_end_point().ToStringWithoutPort());
    }
    http_request->SetHttpHeader("User-Agent", "cnetpp/1.0");
    return c->SendPacket(http_request->ToString());
  };

  http_options.set_connected_callback([&send_func] (HttpConnectionPtr c) -> bool {
      std::cout << "Connected to the server" << std::endl;
      return send_func(c);
  });

  http_options.set_closed_callback([] (HttpConnectionPtr c) -> bool {
      std::cout << "Connection closed" << std::endl;
      (void) c;
      return true;
  });

  http_options.set_received_callback([&send_func] (HttpConnectionPtr c) -> bool {
      static size_t count = 0;
      auto http_response =
        std::static_pointer_cast<cnetpp::http::HttpResponse>(c->http_packet());
      std::string headers;
      http_response->HttpHeadersToString(&headers);
      std::cout << " headers is:" << std::endl;
      std::cout << headers << std::endl;
      if (http_response->http_body().length() > 0) {
        std::cout << "body: " << http_response->http_body() << std::endl;
      }
      if (count++ == 100000) {
        c->MarkAsClosed();
        return true;
      } else {
        return send_func(c);
      }
  });

  http_options.set_sent_callback([] (bool success, HttpConnectionPtr c) -> bool {
    (void) c;
    std::cout << "send packet successfully" << std::endl;
    return success;
  });

  cnetpp::http::HttpClient http_client;
  if (!http_client.Launch(1)) {
    std::cout << "failed to launch the http_client" << std::endl;
    return 1;
  }

  for (auto i = 0; i < 10; ++i) {
    auto connection_id = http_client.Connect(argv[1], http_options);
    if (connection_id == cnetpp::tcp::kInvalidConnectionId) {
      std::cout << "failed to connect to the server" << std::endl;
      return 1;
    }
  }

  sleep(1000);

  http_client.Shutdown();

  return 0;
}

