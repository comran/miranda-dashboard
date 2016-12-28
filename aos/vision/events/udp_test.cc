#include "gtest/gtest.h"
#include "aos/vision/events/udp.h"

namespace aos {
namespace vision {

struct test_data {
  int a;
  int b;
  int c;
  int d;
  void expect_eq(const test_data& other) {
    EXPECT_EQ(a, other.a);
    EXPECT_EQ(b, other.b);
    EXPECT_EQ(c, other.c);
    EXPECT_EQ(d, other.d);
  }
};

TEST(UDPTest, SendRecv) {
  RXUdpSocket rx;
  TXUdpSocket tx;
  rx.Bind(1109);

  test_data txdata { 1, 2, 3, 4};
  test_data rxdata { 0, 0, 0, 0};
  tx.Connect("127.0.0.1", 1109);
  tx.Send((char*)&txdata, sizeof(test_data));
  rx.Recv((char*)&rxdata, sizeof(test_data));
  txdata.expect_eq(rxdata);
}

}  // vision
}  // aos
