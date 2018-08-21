#include <gtest/gtest.h>

namespace frequentSubgraph {
  using std::string;

  class FrequentSubgraphTest : public ::testing::TestWithParam<string> {
  protected:
    FrequentSubgraphTest();
  };
}


