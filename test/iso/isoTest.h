#include <gtest/gtest.h>

namespace isotest {
  using std::string;

  class IsoTest : public ::testing::TestWithParam<string> {
  protected:
    IsoTest();
  };
}


