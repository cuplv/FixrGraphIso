#include <vector>
#include <string>

#include "fixrgraphiso/searchLattice.h"
#include "fixrgraphiso/isomorphismClass.h"


namespace fixrgraphiso {
  using std::vector;

  bool SearchLattice::subsumes(AcdfgBin* acdfgBin) {
    IsoSubsumption d(query,
                     acdfgBin->getRepresentative());
    return d.check();
  }

  bool SearchLattice::isSubsumed(AcdfgBin* acdfgBin) {
    IsoSubsumption d(acdfgBin->getRepresentative(),
                     query);
    return d.check();
  }

  void SearchLattice::findAnomalous(AcdfgBin* popBin,
                                    vector<SearchResult*> results) {
    /* Just add the corrrect subsumed pattern once */
    bool is_correct_subsumed = false;

    /* get all the reachable anomalous pattern that are
       subsumed by the popular one.
    */
    for(AcdfgBin* subsuming : popBin->getSubsumingBins()) {
      if (subsuming->isAnomalous()) {
        if (subsumes(subsuming) && ! is_correct_subsumed) {
          SearchResult* r = new SearchResult(CORRECT_SUBSUMED);
          r->setReferencePattern(popBin);

          results.push_back(r);
          is_correct_subsumed = true;
        } else if (isSubsumed(subsuming)) {
          SearchResult* r = new SearchResult(ANOMALOUS_SUBSUMED);
          r->setReferencePattern(popBin);
          r->setReferencePattern(subsuming);
          results.push_back(r);
        }
      }
    }
  }

  void SearchLattice::search(vector<SearchResult*> results) {
    bool can_subsume = true;
    bool can_be_subsumed = true;

    // Search the relative position of the query w.r.t.
    // the popular pattern
    for (auto it = lattice->beginPopular();
         it != lattice->endPopular(); it++) {
      AcdfgBin* popBin = *it;

      if (can_subsume && subsumes(popBin)) {
        SearchResult* r = new SearchResult(CORRECT);
        r->setReferencePattern(popBin);
        results.push_back(r);

        /* cannot be subsumed by another popular pattern */
        can_be_subsumed = false;
      }

      if (can_be_subsumed && isSubsumed(popBin)) {
        /* search for anomalous patterns */
        findAnomalous(popBin, results);

        /* cannot subsume another popular pattern */
        can_subsume = false;
      }

    }
  }

}
