#include <fstream>
#include <iostream>
#include "frequentSubgraphTest.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/frequentSubgraphs.h"
#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/serializationLattice.h"
#include "fixrgraphiso/searchLattice.h"

namespace frequentSubgraph {
  using std::string;
  using std::cout;
  using std::ifstream;
  using std::ofstream;
  using std::fstream;

  using namespace std;

  using fixrgraphiso::Acdfg;
  using fixrgraphiso::MethodNode;
  using fixrgraphiso::AcdfgSerializer;
  using fixrgraphiso::IsoSubsumption;
  using fixrgraphiso::LatticeSerializer;
  using fixrgraphiso::Lattice;
  using fixrgraphiso::SearchLattice;
  using fixrgraphiso::SearchResult;

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
    Lattice lattice;

    miner.mine(lattice, frequency, method_file,
               output_prefix, acdfg_list);

    {
      LatticeSerializer s;
      string const& outFile = "/tmp/out.serialization.bin";
      iso_protobuf::Lattice * proto = s.proto_from_lattice(lattice);
      fstream myfile(outFile.c_str(), ios::out | ios::binary | ios::trunc);
      proto->SerializeToOstream(&myfile);
      myfile.close();

      proto = s.read_protobuf(outFile.c_str());
      s.lattice_from_proto(proto);
      delete(proto);
    }

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

  TEST_F(FrequentSubgraphTest, LatticeSerialization) {
    string const& inFile = "../test_data/subgraph_results/lattice.bin";

    LatticeSerializer s;

    Lattice *orig;
    Lattice *read;

    {
      iso_protobuf::Lattice * proto = s.read_protobuf(inFile.c_str());
      if (NULL == proto) {
        FAIL() << "Cannot read " + inFile;
      }

      orig = s.lattice_from_proto(proto);
      delete(proto);
    }

    {
      iso_protobuf::Lattice * proto =
        s.proto_from_lattice((const Lattice&) *orig);

      read = s.lattice_from_proto(proto);
      delete(proto);
    }

    delete(orig);
    delete(read);
  }

  TEST_F(FrequentSubgraphTest, LatticeSearch) {
    string const& queryFile =
      "../test_data/com.dagwaging.rosewidgets.db.widget.UpdateService_update.acdfg.bin";
    string const& inFile = "../test_data/subgraph_results/lattice.bin";

    Acdfg* query;
    Lattice *lattice;

    {
      AcdfgSerializer s;
      iso_protobuf::Acdfg * proto =
        s.read_protobuf_acdfg(queryFile.c_str());
      if (NULL == proto)
        FAIL() << "Cannot read " + queryFile;
      query = s.create_acdfg((const iso_protobuf::Acdfg&) *proto);
      delete(proto);
    }

    {
      LatticeSerializer s;
      iso_protobuf::Lattice * proto = s.read_protobuf(inFile.c_str());
      if (NULL == proto)
        FAIL() << "Cannot read " + inFile;
      lattice = s.lattice_from_proto(proto);
      delete(proto);
    }

    {
      SearchLattice searchLattice(query, lattice);
      vector<SearchResult*> results;
      searchLattice.search(results);
    }

    delete(query);
    delete(lattice);
  }
}
