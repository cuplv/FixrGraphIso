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
#include "fixrgraphiso/proto_search.pb.h"

namespace fixrgraphiso {
  using std::vector;
  using std::set;
  using std::ostream;
  namespace fixr_protobuf = edu::colorado::plv::fixr::protobuf;

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
      this->referencePattern = NULL;
      this->anomalousPattern = NULL;
      this->isoToReference = NULL;
      this->isoToAnomalous = NULL;
    }

    void setAnomalousPattern(AcdfgBin* anomalousPattern) {
      this->anomalousPattern = anomalousPattern;
    }

    void setReferencePattern(AcdfgBin* refPat) {
      this->referencePattern = refPat;
    }

    void setIsoToReference(const IsoRepr& iso) {
      this->isoToReference = new IsoRepr(iso);
    }

    void setIsoToAnomalous(const IsoRepr& iso) {
      this->isoToAnomalous = new IsoRepr(iso);
    }

    result_type_t getType() const { return type; }
    AcdfgBin* getReferencePattern() const { return referencePattern;}
    AcdfgBin* getAnomalousPattern() const { return anomalousPattern;}

    IsoRepr* getIsoToReference() const { return isoToReference; }
    IsoRepr* getIsoToAnomalous() const { return isoToAnomalous; }

    static string getTypeRepr(result_type_t type);

  private:
    result_type_t type;
    AcdfgBin* referencePattern;
    AcdfgBin* anomalousPattern;
    IsoRepr* isoToReference;
    IsoRepr* isoToAnomalous;
  };

  /**
   * Implement the search in the lattice
   */
  class SearchLattice {

  private:
    bool subsumes(AcdfgBin* acdfgBin, IsoRepr*& isoRepr);
    bool isSubsumed(AcdfgBin* acdfgBin, IsoRepr*& isoRepr);
    void findAnomalous(AcdfgBin* popBin, const IsoRepr& isoPop,
                       vector<SearchResult*> &results);

  public:
    SearchLattice(Acdfg* query, Lattice* lattice,
                  const bool debug
#ifdef USE_GUROBI_SOLVER
                  , const double gurobi_timeout
#endif
                  ) {
      set<int> ignoreMethodIds;
      this->query = query;
      this->lattice = lattice;
      this->slicedQuery = query->sliceACDFG(lattice->getMethodNames(),
                                            ignoreMethodIds);
      this->debug = debug;
#ifdef USE_GUROBI_SOLVER
      this->gurobi_timeout = gurobi_timeout;
#endif
    }

    ~SearchLattice() {
      delete(slicedQuery);
    }

    void printResult(const vector<SearchResult*> &results,
                     ostream & out_stream);

    fixr_protobuf::SearchResults* toProto(const vector<SearchResult*> &results);

    void search(vector<SearchResult*> &results);
    void newSearch(vector<SearchResult*> & results);

  private:
    Lattice* lattice;
    Acdfg* query;
    Acdfg* slicedQuery;

    bool debug;
#ifdef USE_GUROBI_SOLVER
    double gurobi_timeout;
#endif
  };
}


#endif
