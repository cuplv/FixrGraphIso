#include "isoTest.h"

IsoTest::IsoTest()
{
}

TEST_F(IsoTest, ByDefaultIsoIsTrue) {
  EXPECT_EQ(true, true);
}
