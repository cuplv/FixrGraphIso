#include <fstream>
#include <iostream>
#include "frequentSubgraphTest.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/frequentSubgraphs.h"

namespace frequentSubgraph {
  using std::string;
  using std::cout;
  using std::ifstream;
  using std::ofstream;

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
    string output_prefix = "../test_data/produced_res";
    string method_file = "../test_data/methods_521.txt";
    string acdfg_list = "../test_data/acdfg_list.txt";

    string res_file = "../test_data/subgraph_results/cluster_521_info.txt";

    fixrgraphiso::FrequentSubgraphMiner miner;

    miner.mine(frequency, method_file,
               output_prefix, acdfg_list);

    ifstream res(res_file.c_str());
    string out_file = "cluster-info.txt";
    cout << out_file << endl;
    ifstream myout(out_file.c_str());
    if (! res.is_open()) {
      FAIL() << "Error opening the correct result";
    } else if (! myout.is_open()) {
      FAIL() << "Error opening the test result";
    } else {
      SUCCEED();
    }
  }
}
