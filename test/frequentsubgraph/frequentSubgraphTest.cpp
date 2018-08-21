#include "frequentSubgraphTest.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfg.h"

namespace frequentSubgraph {
  using std::string;
  using std::cout;

  using fixrgraphiso::Acdfg;
  using fixrgraphiso::MethodNode;
  using fixrgraphiso::AcdfgSerializer;
  using fixrgraphiso::IsoSubsumption;

  namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

  FrequentSubgraphTest::FrequentSubgraphTest()
  {
  }

  TEST_P(FrequentSubgraphTest, ByDefaultIsoIsTrue) {
    EXPECT_EQ(true, true);
  }

  INSTANTIATE_TEST_CASE_P(InstantiationName, FrequentSubgraphTest, ::testing::Values(""));
}
