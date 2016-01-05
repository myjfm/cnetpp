#include <gtest/gtest.h>
#include <base/end_point.h>
#include <netinet/in.h>
TEST(EndPoint, SockAddrTest)
{
  sockaddr addr;
  socklen_t addr_len = sizeof(addr);
  cnetpp::base::EndPoint endpoint("127.0.0.1", 80);
  endpoint.ToSockAddr(&addr, &addr_len);
  cnetpp::base::EndPoint endpoint2;
  ASSERT_TRUE(endpoint2.FromSockAddr(addr, addr_len));
  ASSERT_EQ(endpoint.ToString(), endpoint2.ToString());
}
