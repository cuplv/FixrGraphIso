#include <fstream>
#include <iostream>
#include <vector>

#include "searchTest.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/serializationLattice.h"
#include "fixrgraphiso/searchLattice.h"
#include "fixrgraphiso/findDuplicates.h"

namespace search {
  using namespace std;
  using namespace fixrgraphiso;

  SearchTest::SearchTest()
  {
  }

  void searchQuery(Lattice *lattice, Acdfg* query, vector<SearchResult*> &results) {
#ifdef USE_GUROBI_SOLVER
    SearchLattice searchLattice(query, lattice, false, 30);
#else
    SearchLattice searchLattice(query, lattice, false);
#endif
    searchLattice.newSearch(results);

    cout << "Result from search" << endl;
    for (auto res : results) {
      cout << "RES type:" <<
        SearchResult::getTypeRepr(res->getType()) << ": ";
      res->getReferencePattern()->getRepr(cout);
    }
    cout << endl;
  }

  void delRes(vector<SearchResult*> &results) {
    for (auto res : results) {
      delete res;
    }
    results.clear();
  }


  void check_count(const vector<SearchResult*> &results,
                   int totCorrect,
                   int totCorrectSubsumed,
                   int totAnomalous) {
    for (auto res : results) {
      switch (res->getType()) {
        case CORRECT:
          totCorrect--;
          break;
        case CORRECT_SUBSUMED:
          totCorrectSubsumed--;
          break;
        case ANOMALOUS_SUBSUMED:
          totAnomalous--;
          break;
      default:
        FAIL() << "Unexpected result type " << SearchResult::getTypeRepr(res->getType());
      }
    }

    if (totCorrect != 0)
      FAIL() << "Wrong number of correct results";
    if (totCorrectSubsumed != 0)
      FAIL() << "Wrong number of correct subsumed results";
    if (totAnomalous != 0)
      FAIL() << "Wrong number of anomalous results";
  }

  TEST_F(SearchTest, FindAnomaly) {
    string latticeFileName = "../search_data/lattice.bin";
    string queryFileName1 = "../search_data/com.example.tomek.notepad.MainActivity_j.acdfg.bin";
    string queryFileName2 = "../search_data/app.varlorg.unote.RestoreDbActivity_onContextItemSelected.acdfg.bin";
    string queryFileName3 = "../search_data/org.cry.otp.Profiles_onCreateDialog.acdfg.bin";

    Lattice *lattice;
    lattice = fixrgraphiso::readLattice(latticeFileName);

    if (NULL == lattice) {
      FAIL() << "Cannot read the lattice in " << latticeFileName << endl;
    }

    Acdfg* query1 = fixrgraphiso::readAcdfg(queryFileName1);
    if (NULL == query1) {
      delete(lattice);
      FAIL() << "Cannot read query " << queryFileName1 << endl;
    }

    Acdfg* query2 = fixrgraphiso::readAcdfg(queryFileName2);
    if (NULL == query2) {
      delete(query1);
      delete(lattice);
      FAIL() << "Cannot read query " << queryFileName2 << endl;
    }

    Acdfg* query3 = fixrgraphiso::readAcdfg(queryFileName3);
    if (NULL == query3) {
      delete(query1);
      delete(query2);
      delete(lattice);
      FAIL() << "Cannot read query " << queryFileName3 << endl;
    }

    {
      vector<SearchResult*> results;
      SearchResult* res;

      searchQuery(lattice, query1, results);
      if (results.size() != 1)
        FAIL() << "Q1: Unexpected number of results " << results.size() << " instead of 1" << endl;
      res = results.back();
      if (res->getType() != ANOMALOUS_SUBSUMED)
        FAIL() << "Q1: Wrong result type" << endl;
      delRes(results);

      searchQuery(lattice, query2, results);
      if (results.size() != 4)
        FAIL() << "Q2: Unexpected number of results " << results.size() << " instead of 4" << endl;
      check_count(results, 1, 1, 2);
      delRes(results);

      searchQuery(lattice, query3, results);
      if (results.size() != 5)
        FAIL() << "Q3: Unexpected number of results " << results.size() << " instead of 5" << endl;
      check_count(results, 1, 4, 0);
      delRes(results);
    }

    delete(lattice);
    delete(query1);
    delete(query2);
    delete(query3);

    SUCCEED();
  }

  TEST_F(SearchTest, FindDuplicates) {
    string latticeFileName = "../search_data/lattice.bin";
    int popular_bins;
    dup_tuple identicalBins;

    {
      Lattice *lattice;
      lattice = fixrgraphiso::readLattice(latticeFileName);

      if (NULL == lattice) {
        FAIL() << "Cannot read the lattice in " << latticeFileName << endl;
      }

      popular_bins = std::distance(lattice->beginPopular(),
                                   lattice->endPopular());
      delete lattice;
    }

    int res = fixrgraphiso::findDuplicates(latticeFileName, 1,
                                           latticeFileName, 1,
                                           identicalBins);

    if (res != 0)
      FAIL() << "Error finding duplicates" << endl;
    if (identicalBins.size() != popular_bins)
      FAIL() << "Expecting " << popular_bins << " found " <<
        identicalBins.size() << endl;

    for (auto tuple : identicalBins) {
      if (get<0>(tuple) != get<2>(tuple) ||
          get<1>(tuple) != get<3>(tuple)) {
        FAIL() << "Found wrong tuple of identical bins ((" <<
          get<0>(tuple) << ", " << get<1>(tuple) <<
          ",),(" <<
          get<2>(tuple) << ", " << get<3>(tuple) <<
          "))" << endl;
      }
    }

    SUCCEED();
  }
}
