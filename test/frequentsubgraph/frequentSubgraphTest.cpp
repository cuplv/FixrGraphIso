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
  using namespace std;

  using std::string;
  using std::cout;
  using std::ifstream;
  using std::ofstream;
  using std::fstream;
  using std::vector;

  using fixrgraphiso::Acdfg;
  using fixrgraphiso::AcdfgBin;
  using fixrgraphiso::AcdfgSerializer;
  using fixrgraphiso::LatticeSerializer;
  using fixrgraphiso::Lattice;
  using fixrgraphiso::SearchLattice;
  using fixrgraphiso::SearchResult;
  //using fixrgraphiso::CORRECT;
  // using fixrgraphiso::CORRECT_ANOMALOUS;
  // using fixrgraphiso::ANOMALOUS_SUBSUMED;

  namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

  FrequentSubgraphTest::FrequentSubgraphTest()
  {
  }

  void testBinSize(const int expected,
                   const std::vector<AcdfgBin*> vectorBin,
                   const string desc) {
    int vectorSize = vectorBin.size();
    if (expected != vectorSize)
      FAIL() << "Wrong number of " << desc << " bins: " <<
        vectorSize << " instead of " << expected;
  }

  Lattice* readLattice(string latticeFile) {
    LatticeSerializer s;
    Lattice * res = NULL;

    iso_protobuf::Lattice * proto = s.read_protobuf(latticeFile.c_str());
    if (NULL != proto) {
      res = s.lattice_from_proto(proto);
    }
    delete(proto);
    return res;
  }

  Acdfg* readAcdfg(string acdfgPath) {
    AcdfgSerializer s;
    Acdfg* res = NULL;
    iso_protobuf::Acdfg * proto =
      s.read_protobuf_acdfg(acdfgPath.c_str());
    if (NULL != proto) {
      res = s.create_acdfg((const iso_protobuf::Acdfg&) *proto);
      delete(proto);
    }
    return res;
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
      /* check the results */
      testBinSize(47, lattice.getAllBins(), "all");
      testBinSize(3, lattice.getPopularBins(), "popular");
      testBinSize(4, lattice.getAnomalousBins(), "anomalous");
      testBinSize(15, lattice.getIsolatedBins(), "isolated");
    }

    lattice.dumpToDot("/tmp/app2.dot", false);
    lattice.dumpToDot("/tmp/app2_classified.dot", true);

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

    /* test read */
    orig = readLattice(inFile);

    /* test write and read back */
    if (NULL != orig) {
      LatticeSerializer s;
      iso_protobuf::Lattice * protoWrite =
        s.proto_from_lattice((const Lattice&) *orig);

      if (NULL != protoWrite) {
        Lattice *read;

        string const& outFile =
          "../test_data/subgraph_results/lattice_test.bin";
        fstream myfile(outFile.c_str(), ios::out | ios::binary | ios::trunc);
        protoWrite->SerializeToOstream(&myfile);
        myfile.close();
        delete(protoWrite);

        read = readLattice(outFile);
        if (NULL != read) {
          delete(read);
        }
      }
      delete(orig);
    } else {
      FAIL() << "Cannot read the lattice in " << inFile;
    }
  }

  TEST_F(FrequentSubgraphTest, LatticeSearch) {
    string const& inFile = "../test_data/subgraph_results/lattice.bin";
    Lattice *lattice;

    /* Read the searialized lattice */
    lattice = readLattice(inFile);

    if (NULL == lattice) {
      FAIL() << "Cannot read the lattice in " << inFile;
    } else {
      /*
        Input: acdfg in popular pattern
        Output: SearchResult(CORRECT)
                referencePattern: popular bin containing popular
      */
      {
        string const& queryFile =
          "../test_data/com.dagwaging.rosewidgets.db.widget.UpdateService_update.acdfg.bin";

        Acdfg* query = readAcdfg(queryFile);
        if (NULL == query) {
          FAIL() << "Cannot read acdfg " << queryFile;
        } else {
          vector<SearchResult*> results;
          SearchLattice searchLattice(query, lattice);
          searchLattice.search(results);

          ASSERT_EQ(1, results.size()) << "Wrong number of results";
          SearchResult *res = results.front();
          ASSERT_EQ(res->getType(), fixrgraphiso::CORRECT) << "Wrong result type";
          AcdfgBin* ref = res->getReferencePattern();
          ASSERT_TRUE(NULL != ref) << "No reference pattern!";
          ASSERT_TRUE(ref->isACDFGEquivalent(query)) << "Pattern not equivalent";

          delete(query);
        }
      }

      /*
        Input: acdfg from popular, less a method call from important methods
        Output:
        SearchResult(CORRECT_SUBSUMED), referencePattern: popular bin subsuming acdfg, no anomalous subsuming acdfg
      */

      /*
        Input: acdfg from popular, less a method call from important methods
        Output:
        SearchResult(ANOMALOUS_SUBSUMED), referencePattern: popular bin subsuming acdfg, anomalousPattern: anomalous pattern subsuming the acdfg
      */
    }

    delete(lattice);
  }
}
