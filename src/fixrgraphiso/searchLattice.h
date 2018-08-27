/*
 */
#ifndef D__SEARCH_LATTICE__H_
#define D__SEARCH_LATTICE__H_

#include <vector>
#include <map>
#include <string>
#include <set>
#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {
  using std::vector;
  using std::set;
  using std::ostream;

  /**
   * Type of search results inferred from the position of the
   * query in the lattice structure.
   */
  typedef enum {CORRECT,
                CORRECT_SUBSUMED,
                ANOMALOUS_SUBSUMED,
                ISOLATED_SUBSUMED,
                ISOLATED_SUBSUMING} result_type_t;

  /**
   * Single result
   */
  class SearchResult {
  public:
    SearchResult(result_type_t type) {
      this->type = type;
      this->anomalousPattern = NULL;
    }

    void setAnomalousPattern(AcdfgBin* anomalousPattern) {
      this->anomalousPattern = anomalousPattern;
    }

    void setReferencePattern(AcdfgBin* refPat) {
      this->referencePattern = refPat;
    }

    result_type_t getType() { return type; }
    AcdfgBin* getReferencePattern() { return referencePattern;}
    AcdfgBin* getAnomalousPattern() { return anomalousPattern;}

  private:
    result_type_t type;
    AcdfgBin* referencePattern;
    AcdfgBin* anomalousPattern;
    // Add the isomorphism relation to the result
  };

  /**
   * Implement the search in the lattice
   */
  class SearchLattice {

  private:
    bool subsumes(AcdfgBin* acdfgBin);
    bool isSubsumed(AcdfgBin* acdfgBin);
    void findAnomalous(AcdfgBin* popBin, vector<SearchResult*> &results);

  public:
    SearchLattice(Acdfg* query, Lattice* lattice) {
      set<int> ignoreMethodIds;
      this->query = query;
      this->lattice = lattice;
      this->slicedQuery = query->sliceACDFG(lattice->getMethodNames(),
                                            ignoreMethodIds);
    }

    ~SearchLattice() {
      delete(slicedQuery);
    }

    void printResult(const vector<SearchResult*> &results,
                     ostream & out_stream);

    void search(vector<SearchResult*> &results);

  private:
    Lattice* lattice;
    Acdfg* query;
    Acdfg* slicedQuery;
  };
}


#endif