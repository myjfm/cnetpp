#include <gtest/gtest.h>
#include <base/uri.h>

TEST(Uri, ParseTest)
{
  cnetpp::base::Uri uri;
  uri.Parse("http://www.baidu.com");
  ASSERT_EQ(uri.Port(), 80);
  ASSERT_EQ(uri.Scheme(), "http");
  uri.Parse("https://www.baidu.com");
  ASSERT_EQ(uri.Port(), 443);
  ASSERT_EQ(uri.Scheme(), "https");
  uri.Parse("https://www.baidu.com:457");
  ASSERT_EQ(uri.Port(), 457);
  ASSERT_EQ(uri.Host(), "www.baidu.com");
  ASSERT_EQ(uri.Hostname(), "www.baidu.com");
  ASSERT_EQ(uri.Authority(), "www.baidu.com:457");
  ASSERT_EQ(uri.String(), "https://www.baidu.com:457");
  uri.Parse("ftp://www.baidu.com");
  ASSERT_EQ(uri.String(), "ftp://www.baidu.com");
  ASSERT_EQ(uri.Port(), 0);
}
