#include <sys/uio.h>

#include <string>

#include <gtest/gtest.h>

#include <tcp/ring_buffer.h>

TEST(RingBuffer, GetWritePositionsAndCommitWrite) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  struct iovec write_positions[2];
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(10, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abc", 3);
  rb.CommitWrite(3);
  ASSERT_EQ(3, rb.Size());
  ASSERT_EQ(3, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  write_positions[0].iov_len = 0;
  write_positions[1].iov_len = 0;
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(7, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abc", 3);
  rb.CommitWrite(3);
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  write_positions[0].iov_len = 0;
  write_positions[1].iov_len = 0;
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(4, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abcd", 4);
  rb.CommitWrite(4);
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  write_positions[0].iov_len = 0;
  write_positions[1].iov_len = 0;
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(0, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
}

TEST(RingBuffer, GetReadPositionsAndCommitRead) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  struct iovec write_positions[2];
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(10, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abcdefghij", 10);
  rb.CommitWrite(10);
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  struct iovec read_positions[2];
  rb.GetReadPositions(read_positions, 2);
  ASSERT_EQ(10, read_positions[0].iov_len);
  ASSERT_EQ(0, read_positions[1].iov_len);
  ASSERT_EQ(0, ::memcmp(read_positions[0].iov_base, "abcdefghij", 10));
  rb.CommitRead(10);
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  write_positions[0].iov_len = 0;
  write_positions[1].iov_len = 0;
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(10, write_positions[0].iov_len);
  ASSERT_EQ(0, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abcde", 5);
  rb.CommitWrite(5);
  ASSERT_EQ(5, rb.Size());
  ASSERT_EQ(5, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  read_positions[0].iov_len = 0;
  read_positions[1].iov_len = 0;
  rb.GetReadPositions(read_positions, 2);
  ASSERT_EQ(5, read_positions[0].iov_len);
  ASSERT_EQ(0, read_positions[1].iov_len);
  ASSERT_EQ(0, ::memcmp(read_positions[0].iov_base, "abcde", 5));
  rb.CommitRead(5);
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  write_positions[0].iov_len = 0;
  write_positions[1].iov_len = 0;
  rb.GetWritePositions(write_positions, 2);
  ASSERT_EQ(5, write_positions[0].iov_len);
  ASSERT_EQ(5, write_positions[1].iov_len);
  ::memcpy(write_positions[0].iov_base, "abcde", 5);
  ::memcpy(write_positions[1].iov_base, "f", 1);
  rb.CommitWrite(6);
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  read_positions[0].iov_len = 0;
  read_positions[1].iov_len = 0;
  rb.GetReadPositions(read_positions, 2);
  ASSERT_EQ(5, read_positions[0].iov_len);
  ASSERT_EQ(1, read_positions[1].iov_len);
  ASSERT_EQ(0, ::memcmp(read_positions[0].iov_base, "abcde", 5));
  ASSERT_EQ(0, ::memcmp(read_positions[1].iov_base, "f", 1));
  rb.CommitRead(6);
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
}

TEST(RingBuffer, WriteAndRead) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcde", 5);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(5, rb.Size());
  ASSERT_EQ(5, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abcdef", 6);
  ASSERT_FALSE(rb.Write(data));
  ASSERT_EQ(5, rb.Size());
  ASSERT_EQ(5, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  data.set("abcde", 5);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 2));
  ASSERT_EQ("ab", buffer);
  ASSERT_EQ(8, rb.Size());
  ASSERT_EQ(8, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Read(&buffer, 5));
  ASSERT_EQ("abcdeab", buffer);
  ASSERT_EQ(3, rb.Size());
  ASSERT_EQ(3, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abcdef", 6);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(9, rb.Size());
  ASSERT_EQ(9, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Read(&buffer, 5));
  ASSERT_EQ("abcdeabcdeab", buffer);
  ASSERT_EQ(4, rb.Size());
  ASSERT_EQ(4, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_FALSE(rb.Read(&buffer, 5));
  ASSERT_EQ(4, rb.Size());
  ASSERT_EQ(4, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Read(&buffer, 4));
  ASSERT_EQ("abcdeabcdeabcdef", buffer);
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
}

TEST(RingBuffer, FindStringAndReform1) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 6));
  ASSERT_EQ("abcdef", buffer);
  ASSERT_EQ(4, rb.Size());
  ASSERT_EQ(4, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abc", 3);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(7, rb.Size());
  ASSERT_EQ(7, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Find("ja", &data));
  ASSERT_EQ(7, rb.Size());
  ASSERT_EQ(7, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("ghi", data.as_string());
}

TEST(RingBuffer, FindCharAndReform1) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 6));
  ASSERT_EQ("abcdef", buffer);
  ASSERT_EQ(4, rb.Size());
  ASSERT_EQ(4, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abc", 3);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(7, rb.Size());
  ASSERT_EQ(7, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Find('b', &data));
  ASSERT_EQ(7, rb.Size());
  ASSERT_EQ(7, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("ghija", data.as_string());
}

TEST(RingBuffer, FindStringAndReform2) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 7));
  ASSERT_EQ("abcdefg", buffer);
  ASSERT_EQ(3, rb.Size());
  ASSERT_EQ(3, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abc", 3);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Find("ja", &data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("hi", data.as_string());

  ASSERT_TRUE(rb.Find('b', &data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("hija", data.as_string());
}

TEST(RingBuffer, FindCharAndReform2) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 7));
  ASSERT_EQ("abcdefg", buffer);
  ASSERT_EQ(3, rb.Size());
  ASSERT_EQ(3, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abc", 3);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Find('b', &data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("hija", data.as_string());
}

TEST(RingBuffer, FindStringAndReform3) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 8));
  ASSERT_EQ("abcdefgh", buffer);
  ASSERT_EQ(2, rb.Size());
  ASSERT_EQ(2, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abcdef", 6);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(8, rb.Size());
  ASSERT_EQ(8, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_TRUE(rb.Find("jab", &data));
  ASSERT_EQ(8, rb.Size());
  ASSERT_EQ(8, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("i", data.as_string());

  ASSERT_TRUE(rb.Find('c', &data));
  ASSERT_EQ(8, rb.Size());
  ASSERT_EQ(8, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  ASSERT_EQ("ijab", data.as_string());
}

TEST(RingBuffer, FindCharAndReform3) {
  cnetpp::tcp::RingBuffer rb(10);
  ASSERT_TRUE(rb.Empty());
  ASSERT_FALSE(rb.Full());
  ASSERT_EQ(0, rb.Size());
  ASSERT_EQ(0, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
  cnetpp::base::StringPiece data("abcdefghij", 10);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(10, rb.Size());
  ASSERT_EQ(10, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  std::string buffer;
  ASSERT_TRUE(rb.Read(&buffer, 7));
  ASSERT_EQ("abcdefg", buffer);
  ASSERT_EQ(3, rb.Size());
  ASSERT_EQ(3, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  data.set("abc", 3);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());

  ASSERT_FALSE(rb.Find("jad", &data));
  ASSERT_EQ(6, rb.Size());
  ASSERT_EQ(6, rb.Length());
  ASSERT_EQ(10, rb.Capacity());
}

TEST(RingBuffer, FindCharAndReform4) {
  cnetpp::tcp::RingBuffer rb(500);
  cnetpp::base::StringPiece data("41\nabcdefghijklmn", 17);
  ASSERT_TRUE(rb.Write(data));
  ASSERT_EQ(17, rb.Size());
  ASSERT_EQ(17, rb.Length());
  ASSERT_EQ(500, rb.Capacity());

  cnetpp::base::StringPiece result;
  ASSERT_TRUE(rb.Find('\n', &result));
  ASSERT_EQ(2, result.size());
  ASSERT_EQ("41", result.as_string());
}

