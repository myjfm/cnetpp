#include <cnetpp/base/end_point.h>
#include <cnetpp/base/ip_address.h>
#include <cnetpp/base/string_utils.h>
#include <cnetpp/base/log.h>
#include <cnetpp/http/http_connection.h>
#include <cnetpp/http/http_request.h>
#include <cnetpp/http/http_response.h>
#include <cnetpp/http/http_server.h>

#include <unistd.h>

#include <iostream>
#include <memory>

class HttpServerHandler final {
 public:
  HttpServerHandler() = default;
  ~HttpServerHandler() = default;

  bool OnConnected(std::shared_ptr<cnetpp::http::HttpConnection> c) {
    INFO("A new connection arrived");
    assert(c.get());
    (void) c;
    return true;
  }

  bool OnReceived(std::shared_ptr<cnetpp::http::HttpConnection> c) {
    assert(c.get());
    auto http_request =
        static_cast<cnetpp::http::HttpRequest*>(c->http_packet().get());
    if (!http_request) {
      return false;
    }
    INFO("uri: %s", http_request->uri().c_str());
    INFO("method: %d", static_cast<int>(http_request->method()));
    std::shared_ptr<cnetpp::http::HttpResponse> http_response(
        new cnetpp::http::HttpResponse);
    http_response->set_status(
        static_cast<cnetpp::http::HttpResponse::StatusCode>(200));
    http_response->SetHttpHeader("Content-Length", "10");
    http_response->set_http_body("1234567890");
    std::string str_response;
    http_response->ToString(&str_response);
    c->SendPacket(str_response);
    //c->MarkAsClosed(false);
    return true;
  }

  bool OnClosed(std::shared_ptr<cnetpp::http::HttpConnection> c) {
    assert(c.get());
    (void) c;
    INFO("Connection closed");
    return true;
  }

  bool OnSent(bool success, std::shared_ptr<cnetpp::http::HttpConnection> c) {
    (void) c;
    INFO("Message has been sent");
    return success;
  }
};

int main() {
  cnetpp::base::IPAddress listen_ip("127.0.0.1");
  cnetpp::base::EndPoint listen_end_point(listen_ip, 12346);

  HttpServerHandler http_handler;
  cnetpp::http::HttpServerOptions options;
  options.set_connected_callback(
      [&http_handler] (
        std::shared_ptr<cnetpp::http::HttpConnection> c) -> bool {
      return http_handler.OnConnected(c);
  });
  options.set_closed_callback(
      [&http_handler] (
        std::shared_ptr<cnetpp::http::HttpConnection> c) -> bool {
      return http_handler.OnClosed(c);
  });
  options.set_received_callback(
      [&http_handler] (
        std::shared_ptr<cnetpp::http::HttpConnection> c) -> bool {
      return http_handler.OnReceived(c);
  });
  options.set_sent_callback(
      [&http_handler] (
        bool success,
        std::shared_ptr<cnetpp::http::HttpConnection> c) -> bool {
      return http_handler.OnSent(success, c);
  });

  cnetpp::http::HttpServer http_server;
  if (!http_server.Launch(listen_end_point, options, 1)) {
    FATAL("Failed to launch http server, exiting...");
  }

  ::sleep(1000);

  return 0;
}

