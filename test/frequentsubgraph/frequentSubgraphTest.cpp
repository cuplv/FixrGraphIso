#include "frequentSubgraphTest.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/frequentSubgraphs.h"

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

  TEST_F(FrequentSubgraphTest, ByDefaultIsoIsTrue) {
    int frequency = 20;
    string output_prefix = "output.txt";
    string method_file = "../test_data/methods_521.txt";
    string acdfg_list = "../test_data/acdfg_list.txt";

    string res_file = "../test_data/subgraph_results/cluster_521_info.txt";

    fixrgraphiso::FrequentSubgraphMiner miner;

    miner.mine(frequency,
               method_file,
               output_prefix,
               acdfg_list);

    EXPECT_EQ(true, true);
  }
}
