#include <unistd.h>

#include <iostream>

#include "http/http_callbacks.h"
#include "http/http_client.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include "base/end_point.h"
#include "base/ip_address.h"

class HttpClientHandler final {
 public:
  HttpClientHandler() = default;
  ~HttpClientHandler() = default;

  using HttpConnectionPtr = std::shared_ptr<cnetpp::http::HttpConnection>;
  bool OnConnected(HttpConnectionPtr http_connection) {
    std::cout << "Connected to the server" << std::endl;
    std::shared_ptr<cnetpp::http::HttpRequest> http_request(
        new cnetpp::http::HttpRequest);
    http_request->set_method(cnetpp::http::HttpRequest::MethodType::kGet);
    http_request->set_uri("/");
    //http_request->SetHttpHeader("Content-Length", "0");
    http_request->SetHttpHeader("Host", "www.qq.com");
    http_request->SetHttpHeader("User-Agent", "cnetpp/1.0");
    return http_connection->SendPacket(http_request->ToString());
  }
  bool OnClosed(HttpConnectionPtr http_connection) {
    std::cout << "Connection closed" << std::endl;
    (void) http_connection;
    return true;
  }
  bool OnReceived(HttpConnectionPtr http_connection) {
    auto http_response = std::static_pointer_cast<cnetpp::http::HttpResponse>(
        http_connection->http_packet());
    std::string headers;
    http_response->HttpHeadersToString(&headers);
    std::cout << " headers is:" << std::endl;
    std::cout << headers << std::endl;
    if (http_response->http_body().length() > 0) {
      std::cout << "body: " << http_response->http_body() << std::endl;
    }
    http_connection->MarkAsClosed();
    return true;
  }
  bool OnSent(bool success, HttpConnectionPtr http_connection) {
    (void) http_connection;
    std::cout << "send packet successfully" << std::endl;
    return success;
  }
};

int main(int argc, const char **argv) {
  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
    return 1;
  }
  using HttpConnectionPtr = std::shared_ptr<cnetpp::http::HttpConnection>;
  HttpClientHandler http_client_handler;
  cnetpp::http::HttpClient http_client;
  cnetpp::http::HttpOptions http_options;
  http_options.set_send_buffer_size(1024 * 1024);
  http_options.set_receive_buffer_size(1024 * 1024);
  http_options.set_connected_callback(
      [&http_client_handler] (HttpConnectionPtr c) -> bool {
        return http_client_handler.OnConnected(c);
      }
  );
  http_options.set_closed_callback(
      [&http_client_handler] (HttpConnectionPtr c) -> bool {
        return http_client_handler.OnClosed(c);
      }
  );
  http_options.set_received_callback(
      [&http_client_handler] (HttpConnectionPtr c) -> bool {
        return http_client_handler.OnReceived(c);
      }
  );
  http_options.set_sent_callback(
      [&http_client_handler] (bool success, HttpConnectionPtr c) -> bool {
        return http_client_handler.OnSent(success, c);
      }
  );
  //cnetpp::base::IPAddress remote_ip("127.0.0.1");
  //cnetpp::base::EndPoint remote_end_point(remote_ip, 12346);
  //cnetpp::base::IPAddress remote_ip("202.108.33.60");
  //cnetpp::base::EndPoint remote_end_point(remote_ip, 80);
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

  sleep(10);

  http_client.Shutdown();

  return 0;
}
