#include <cnetpp/tcp/tcp_client.h>
#include <cnetpp/tcp/tcp_server.h>
#include <cnetpp/tcp/tcp_connection.h>
#include <cnetpp/tcp/tcp_options.h>
#include <cnetpp/base/log.h>

#include <gtest/gtest.h>

#include <memory>

namespace cnetpp {

class TcpClientTest : public testing::Test {
public:
  void SetUp() override {
    eps_.emplace_back("127.0.0.1", server_port1_);
    eps_.emplace_back("127.0.0.1", server_port2_);

    server1_ = std::make_shared<tcp::TcpServer>();
    tcp::TcpServerOptions tcp_server_options;
    tcp_server_options.set_name("srv1");
    tcp_server_options.set_connected_callback (
            [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
      CnetppDebug("Server [Socket 0X%08x] [%s 0X%08x] connected",
                  c->socket().fd(), c->ToName().c_str(), c->id());
      //CnetppDebug("backTrace: %s", base::BacktraceString().c_str());
      return true;
    });

    tcp_server_options.set_received_callback (
            [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool{
      CnetppDebug("Server [Socket 0X%08x] [%s 0X%08x] received",
                  c->socket().fd(), c->ToName().c_str(), c->id());
      auto &buffer = c->mutable_recv_buffer();
      if(buffer.Size() != 4) return true;
      char client_data[4];
      buffer.Read(client_data, sizeof(client_data));
      client_data[1] = 'o';
      c->SendPacket(base::StringPiece(client_data, sizeof(client_data)));
      return true;
    });

    tcp_server_options.set_sent_callback (
            [&](bool b, std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
      CnetppDebug("Server [Socket 0X%08x] [%s 0X%08x] have sent some data",
                  c->socket().fd(), c->ToName().c_str(), c->id());
      return true;
    });

    tcp_server_options.set_closed_callback (
            [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
      CnetppDebug("Server [Socket 0X%08x] [%s 0X%08x] close",
                  c->socket().fd(), c->ToName().c_str(), c->id());
      return true;
    });

    server1_->Launch(eps_[0], tcp_server_options);

    server2_ = nullptr;
  }

  void TearDown() override {
    server1_->Shutdown();
    //server2_->Shutdown();
  }

  int server_port1_ { 8545 };
  int server_port2_ { 8546 };
  std::shared_ptr<tcp::TcpServer> server1_;
  std::shared_ptr<tcp::TcpServer> server2_;
  std::vector<cnetpp::base::EndPoint> eps_;
  int ping_count_ { 0 };
  std::atomic_int cur_num_ { 0 };
};

TEST_F(TcpClientTest, SimpleConnectionTest) {
  int client_num = 100;
  ping_count_ = 2 * client_num;
  std::atomic_int closed_num { 0 };

  //base::LOG.set_func(base::LOG.DefaultEmptyFunc);

  std::shared_ptr<cnetpp::tcp::TcpClient> client =
          std::make_shared<cnetpp::tcp::TcpClient>();
  cnetpp::tcp::TcpClientOptions *tcp_client_options =
          new cnetpp::tcp::TcpClientOptions();
  tcp_client_options->set_connected_callback (
          [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
    CnetppDebug("Client [Socket 0X%08x] [%s 0X%08x] connected",
                c->socket().fd(), c->ToName().c_str(), c->id());
    //CnetppDebug("backTrace: %s", base::BacktraceString().c_str());
    c->SendPacket("Ping");
    return true;
  });

  tcp_client_options->set_received_callback (
          [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
    CnetppDebug("Client [Socket 0X%08x] [%s 0X%08x] received",
                c->socket().fd(), c->ToName().c_str(), c->id());
    auto &buffer = c->mutable_recv_buffer();
    if(buffer.Size() != 4) return true;
    char client_data[4];
    buffer.Read(client_data, sizeof(client_data));
    assert(strncmp(client_data, "Pong", 4) == 0);
    if (++cur_num_ >= ping_count_) {
      c->MarkAsClosed();
      CnetppDebug("markclose socket for [Socket 0X%08x] [%s 0X%08x]",
                  c->socket().fd(), c->ToName().c_str(), c->id());
      return true;
    }

    c->SendPacket(base::StringPiece(client_data, sizeof(client_data)));
    return true;
  });

  tcp_client_options->set_sent_callback (
          [&](bool b, std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
    CnetppDebug("Client [Socket 0X%08x] [%s 0X%08x] have sent some data",
                c->socket().fd(), c->ToName().c_str(), c->id());
    return true;
  });

  tcp_client_options->set_closed_callback (
          [&](std::shared_ptr<cnetpp::tcp::TcpConnection> c) -> bool {
    CnetppDebug("[Socket 0X%08x] [%s 0X%08x] close",
                c->socket().fd(), c->ToName().c_str(), c->id());
    ++closed_num;
    return true;
  });

  client->Launch("cli1", *tcp_client_options);
  for (int i = 0; i < client_num; ++i) {
    client->Connect(&eps_[0], *tcp_client_options, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  while (closed_num < client_num) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    CnetppDebug("client test sleep 5 seconds");
  }
}

#if 0
TEST_F(StatelessRpcChannelTest, AsyncServerFailed) {
  ASSERT_TRUE(server1_->Shutdown());
  ASSERT_TRUE(server2_->Shutdown());

  int n = 1100;
  CountDownLatch latch(n);
  for (int i = 0; i < n; ++i) {
    auto controller = new RpcController();
    controller->set_retry_count(10);
    auto request = new hadoop::hdfs::EchoRequestProto();
    auto response = new hadoop::hdfs::EchoResponseProto();
    request->set_payload("2222");
    response->Clear();
    client_->echo(controller, request, response,
                  new Closure([&latch, controller, request, response] {
                    // We must count down the latch first, because if ASSERT_XX
                    // failed, there is no chance to count down any more
                    latch.CountDown();
                    ASSERT_EQ(RpcStatus::kNetworkError, controller->status());
                  }));
  }
  latch.Await();
  // In case of main thread quits first.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(StatelessRpcChannelTest, SyncPingPongConcurrently) {
  auto workers = std::make_shared<cnetpp::concurrency::ThreadPool>("");
  workers->set_num_threads(10);
  workers->Start();

  int n = 10000;
  CountDownLatch latch(n);
  for (int i = 0; i < n; i++) {
    workers->AddTask([&, i]() -> bool {
      EchoSuccess(i);
      latch.CountDown();
      return true;
    });
  }

  latch.Await();
}

TEST_F(StatelessRpcChannelTest, SyncServerFailed) {
  ASSERT_TRUE(server1_->Shutdown());
  ASSERT_TRUE(server2_->Shutdown());

  auto workers = std::make_shared<cnetpp::concurrency::ThreadPool>("");
  workers->set_num_threads(10);
  workers->Start();

  int n = 10;
  CountDownLatch latch(n);

  for (int i = 0; i < 10; ++i) {
    workers->AddTask([&, i]() -> bool {
      EchoNetworkError(i);
      latch.CountDown();
      return true;
    });
  }

  latch.Await();
}
#endif

}  // namespace cnetpp
