#include <cnetpp/base/uri.h>

#include <gtest/gtest.h>

TEST(Uri, ParseTest) {
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

  cnetpp::base::Uri uri2;
  ASSERT_TRUE(uri2.ParseUriPath("/test/url?p=%2fa%2fb%2fc-d%2fe&q=%401"));
  auto params = uri2.QueryParams();

  ASSERT_EQ(params.size(), (size_t)2);
  ASSERT_EQ(params[0].first, "p");
  ASSERT_EQ(params[0].second, "/a/b/c-d/e");
  ASSERT_EQ(params[1].first, "q");
  ASSERT_EQ(params[1].second, "@1");

  cnetpp::base::Uri uri3;
  ASSERT_TRUE(uri3.ParseUriPath("/test/url?path=%2Fdms%2Fwo%2Fdms%2Fwba_ant"
        "%2Fd%3D2%2Fh%3D02%3332Fnid%3D200"));
  auto params3 = uri3.QueryParams();
  ASSERT_EQ(params3[0].first, "path");
  ASSERT_EQ(params3[0].second, "/dms/wo/dms/wba_ant/d=2/h=02333/nid=200");
}
